#include "pch.h"
#include "syncprims.h"
#ifdef _WIN32
#include <Psapi.h>
#else
#include <unistd.h>
#endif

// In linux you can use named semaphores for ipc mutex.
// Android doesn't support named semaphores though.
#ifdef _WIN32
PAPI void			AbcMutexCreate( AbcMutex& mutex, LPCSTR name )
{
	mutex = CreateMutexA( NULL, false, name );
}
PAPI void			AbcMutexDestroy( AbcMutex& mutex )
{
	CloseHandle( mutex );
}
PAPI bool			AbcMutexWait( AbcMutex& mutex, DWORD waitMS )
{
	return WaitForSingleObject( mutex, waitMS ) == WAIT_OBJECT_0;
}
PAPI void			AbcMutexRelease( AbcMutex& mutex )
{
	ReleaseMutex( mutex );
}

PAPI void			AbcCriticalSectionInitialize( AbcCriticalSection& cs, unsigned int spinCount )
{
	if ( spinCount != 0 )	AbcVerify( InitializeCriticalSectionAndSpinCount( &cs, spinCount ) );
	else					InitializeCriticalSection( &cs );
}
PAPI void			AbcCriticalSectionDestroy( AbcCriticalSection& cs )
{
	DeleteCriticalSection( &cs );
}
PAPI bool			AbcCriticalSectionTryEnter( AbcCriticalSection& cs )
{
	return !!TryEnterCriticalSection( &cs );
}
PAPI void			AbcCriticalSectionEnter( AbcCriticalSection& cs )
{
	EnterCriticalSection( &cs );
}
PAPI void			AbcCriticalSectionLeave( AbcCriticalSection& cs )
{
	LeaveCriticalSection( &cs );
}

PAPI void			AbcSemaphoreInitialize( AbcSemaphore& sem )
{
	sem = CreateSemaphore( NULL, 0, 0x7fffffff, NULL );
}
PAPI void			AbcSemaphoreDestroy( AbcSemaphore& sem )
{
	CloseHandle( sem );
	sem = NULL;
}
PAPI bool			AbcSemaphoreWait( AbcSemaphore& sem, DWORD waitMS )
{
	return WaitForSingleObject( sem, waitMS ) == WAIT_OBJECT_0;
}
PAPI void			AbcSemaphoreRelease( AbcSemaphore& sem, DWORD count )
{
	ReleaseSemaphore( sem, (LONG) count, NULL );
}

PAPI bool			AbcThreadCreate( AbcThreadFunc threadfunc, void* context, AbcThreadHandle& handle )
{
	handle = CreateThread( NULL, 0, threadfunc, context, 0, NULL );
	return NULL != handle;
}
PAPI bool			AbcThreadJoin( AbcThreadHandle handle )
{
	return WaitForSingleObject( handle, INFINITE ) == WAIT_OBJECT_0;
}
PAPI void			AbcThreadCloseHandle( AbcThreadHandle handle )
{
	CloseHandle( handle );
}
PAPI AbcThreadHandle AbcThreadCurrent()
{
	return GetCurrentThread();
}
PAPI void			AbcThreadSetPriority( AbcThreadHandle handle, AbcThreadPriority priority )
{
	DWORD p = THREAD_PRIORITY_NORMAL;
	switch( priority )
	{
	case AbcThreadPriorityIdle:				p = THREAD_PRIORITY_IDLE; break;
	case AbcThreadPriorityNormal:			p = THREAD_PRIORITY_NORMAL; break;
	case AbcThreadPriorityHigh:				p = THREAD_PRIORITY_HIGHEST; break;
	case AbcThreadPriorityBackgroundBegin:	p = THREAD_MODE_BACKGROUND_BEGIN; break;
	case AbcThreadPriorityBackgroundEnd:	p = THREAD_MODE_BACKGROUND_END; break;
	}
	SetThreadPriority( handle, p );
}
PAPI void			AbcSleep( int milliseconds )
{
	Sleep( milliseconds );	
}
#if XSTRING_DEFINED
PAPI bool			AbcProcessCreate( const char* cmd, AbcForkedProcessHandle* handle, AbcProcessID* pid )
{
	XStringW c = cmd;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	if ( !CreateProcess( NULL, c.GetRawBuffer(), NULL, NULL, false, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi ) )
		return false;
	CloseHandle( pi.hThread );
	if ( handle )
		*handle = pi.hProcess;
	else
		CloseHandle( pi.hProcess );
	if ( pid )
		*pid = pi.dwProcessId;
	return true;
}
#endif
PAPI bool			AbcProcessWait( AbcForkedProcessHandle handle, int* exitCode )
{
	if ( WaitForSingleObject( handle, INFINITE ) != WAIT_OBJECT_0 )
	{
		if ( exitCode )
		{
			DWORD code = 0;
			GetExitCodeProcess( handle, &code );
			*exitCode = code;
		}
		return true;
	}
	return false;
}
PAPI void 			AbcProcessCloseHandle( AbcForkedProcessHandle handle )
{
	CloseHandle( handle );
}
PAPI AbcProcessID	AbcProcessGetPID()
{
	return GetCurrentProcessId();
}
PAPI void			AbcProcessGetPath( wchar_t* path, size_t maxPath )
{
	GetModuleFileNameW( NULL, path, (DWORD) maxPath );
}
PAPI void			AbcEnumProcesses( podvec<AbcProcessID>& pids )
{
	DWORD id_static[1024];
	DWORD* id = id_static;
	DWORD id_size = sizeof(id_static);
	DWORD id_used = 0;
	while ( true )
	{
		id_used = 0;
		EnumProcesses( id, id_size, &id_used );
		if ( id_used == id_size )
		{
			// likely need more space
			if ( id == id_static )
				id = (DWORD*) AbcMallocOrDie( id_size );
			else
				id = (DWORD*) AbcReallocOrDie( id, id_size );
		}
		else
		{
			break;
		}
	}
	pids.resize_uninitialized( id_used / sizeof(DWORD) );
	memcpy( &pids[0], id, id_used );
	if ( id != id_static )
		free(id);
}

PAPI void			AbcMachineInformationGet( AbcMachineInformation& info )
{
	SYSTEM_INFO inf;
	GetSystemInfo( &inf );
	info.CPUCount = inf.dwNumberOfProcessors;
}

#else // End Win32

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Linux
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void MillisecondsToAbsTimespec( DWORD ms, timespec& t )
{
	clock_gettime( CLOCK_REALTIME, &t );
	t.tv_sec += ms / 1000;
	t.tv_nsec += (ms % 1000) * 1000;
	if ( t.tv_nsec >= 1000000 )
	{
		t.tv_sec += 1;
		t.tv_nsec -= 1000000;
	}
}

PAPI void			AbcMutexCreate( AbcMutex& mutex, LPCSTR name )
{
	AbcAssert( name == NULL || name[0] == 0 );
	AbcVerify( 0 == pthread_mutex_init( &mutex, NULL ) );
}
PAPI void			AbcMutexDestroy( AbcMutex& mutex )
{
	AbcVerify( 0 == pthread_mutex_destroy( &mutex ) );
}
PAPI bool			AbcMutexWait( AbcMutex& mutex, DWORD waitMS )
{
	if ( waitMS == 0 )
		return 0 == pthread_mutex_trylock( &mutex );
	else if ( waitMS == AbcINFINITE )
		return 0 == pthread_mutex_lock( &mutex );
	else
	{
		timespec t;
		MillisecondsToAbsTimespec( waitMS, t );
		return 0 == pthread_mutex_timedlock( &mutex, &t );
	}
}
PAPI void			AbcMutexRelease( AbcMutex& mutex )
{
	pthread_mutex_unlock( &mutex );
}

PAPI void			AbcCriticalSectionInitialize( AbcCriticalSection& cs, unsigned int spinCount )
{
	AbcVerify( 0 == pthread_mutex_init( &cs, NULL ) );
}
PAPI void			AbcCriticalSectionDestroy( AbcCriticalSection& cs )
{
	AbcVerify( 0 == pthread_mutex_destroy( &cs ) );
}
PAPI bool			AbcCriticalSectionTryEnter( AbcCriticalSection& cs )
{
	return 0 == pthread_mutex_trylock( &cs );
}
PAPI void			AbcCriticalSectionEnter( AbcCriticalSection& cs )
{
	pthread_mutex_lock( &cs );
}
PAPI void			AbcCriticalSectionLeave( AbcCriticalSection& cs )
{
	pthread_mutex_unlock( &cs );
}

PAPI void			AbcSemaphoreInitialize( AbcSemaphore& sem )
{
	AbcVerify( 0 == sem_init( &sem, 0, 0 ) );
}
PAPI void			AbcSemaphoreDestroy( AbcSemaphore& sem )
{
	AbcVerify( 0 == sem_destroy( &sem ) );
}
PAPI bool			AbcSemaphoreWait( AbcSemaphore& sem, DWORD waitMS )
{
	if ( waitMS == 0 )
		return 0 == sem_trywait( &sem );
	else if ( waitMS == AbcINFINITE )
		return 0 == sem_wait( &sem );
	else
	{
		timespec t;
		MillisecondsToAbsTimespec( waitMS, t );
		return 0 == sem_timedwait( &sem, &t );
	}
}
PAPI void			AbcSemaphoreRelease( AbcSemaphore& sem, DWORD count )
{
	for ( DWORD i = 0; i < count; i++ )
		sem_post( &sem );
}

PAPI bool			AbcThreadCreate( AbcThreadFunc threadfunc, void* context, AbcThreadHandle& handle )
{
	pthread_attr_t attr;
	handle = 0;
	AbcVerify( 0 == pthread_attr_init( &attr ) );
	bool ok = pthread_create( &handle, &attr, threadfunc, context );
	AbcVerify( 0 == pthread_attr_destroy( &attr ) );
	return ok;
}
PAPI bool			AbcThreadJoin( AbcThreadHandle handle )
{
	void* rval;
	return 0 == pthread_join( handle, &rval );
}
PAPI void			AbcThreadCloseHandle( AbcThreadHandle handle )
{
	// pthread has no equivalent/requirement of this
}
PAPI AbcThreadHandle AbcThreadCurrent()
{
	return pthread_self();
}
PAPI void			AbcThreadSetPriority( AbcThreadHandle handle, AbcThreadPriority priority )
{
	// On linux, pthread_setschedprio is only applicable to threads on the realtime scheduling policy
}

PAPI void			AbcSleep( int milliseconds )
{
	int64 nano = milliseconds * (int64) 1000;
	timespec t;
	t.tv_nsec = nano % 1000000000; 
	t.tv_sec = (nano - t.tv_nsec) / 1000000000;
	nanosleep( &t, NULL );
}

PAPI bool			AbcProcessCreate( const char* cmd, AbcForkedProcessHandle* handle, AbcProcessID* pid )
{
	AbcAssert(false);
}
PAPI bool			AbcProcessWait( AbcForkedProcessHandle handle, int* exitCode )
{
	AbcAssert(false);
}
PAPI void			AbcProcessCloseHandle( AbcForkedProcessHandle handle )
{
	AbcAssert(false);
}
PAPI AbcProcessID	AbcProcessGetPID()
{
	return getpid();
}
PAPI void			AbcProcessGetPath( wchar_t* path, size_t maxPath )
{
	AbcAssert(false); // untested code
	if ( maxPath == 0 ) return;
	char buf[256];
	path[0] = 0;
    size_t r = readlink( "/proc/self/exe", buf, arraysize(buf) - 1 );
	if ( r == -1 )
		return;
	buf[r] = 0;
	mbstowcs( path, buf, maxPath - 1 );
}
PAPI void			AbcEnumProcesses( podvec<AbcProcessID>& pids )
{
	AbcAssert(false);
}
PAPI void			AbcMachineInformationGet( AbcMachineInformation& info )
{
	info.CPUCount = sysconf( _SC_NPROCESSORS_ONLN );
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Cross-platform helpers

PAPI void		AbcSpinLockWait( volatile unsigned int* p )
{
	while ( AbcCmpXChg( p, 1, 0 ) != 0 )
	{
	}
}

PAPI void		AbcSpinLockRelease( volatile unsigned int* p )
{
	AbcAssert( *p == 1 );
	AbcInterlockedSet( p, 0 );
}

#if XSTRING_DEFINED
PAPI XString		AbcProcessGetPath()
{
	wchar_t p[MAX_PATH];
	AbcProcessGetPath( p, arraysize(p) );
	p[MAX_PATH - 1] = 0;
	return p;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32
AbcSyncEvent::AbcSyncEvent()
{
	Persistent = false;
	Initialized = false;
}
#endif

AbcSyncEvent::~AbcSyncEvent()
{
	Destroy();
}

void AbcSyncEvent::Initialize( bool persistent )
{
#ifdef _WIN32
	Event = CreateEvent( NULL, persistent, false, NULL );
#else
	Persistent = persistent;
	Initialized = true;
	AbcSemaphoreInitialize( Sem );
#endif
}

void AbcSyncEvent::Destroy()
{
#ifdef _WIN32
	if ( Event != NULL )
		CloseHandle( Event );
	Event = NULL;
#else
	if ( Initialized )
		AbcSemaphoreDestroy( Sem );
	Initialized = false;
#endif
}

void AbcSyncEvent::Signal()
{
#ifdef _WIN32
	AbcAssert( Event != NULL );
	SetEvent( Event );
#else
	AbcAssert( Initialized );
	// When Persistent=true, this is racy (explained in the main comments)
	AbcSemaphoreRelease( Sem, Persistent ? PersistentPostCount : 1 );
#endif
}

bool AbcSyncEvent::Wait( DWORD waitMS )
{
#ifdef _WIN32
	return WaitForSingleObject( Event, waitMS ) == WAIT_OBJECT_0;
#else
	if ( AbcSemaphoreWait( Sem, waitMS ) )
	{
		if ( Persistent )
		{
			// this is racy (explained in the main comments)
			AbcSemaphoreRelease( Sem, 1 );
		}
		return true;
	}
	return false;
#endif
}
