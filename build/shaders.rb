
=begin

Shader Generator for nuDom
==========================

Input is pairs of .glsl files inside nudom/Shaders, such as "RectFrag.glsl", "RectVert.glsl".
Output is 2 files per shader, such as "RectShader.cpp", "RectShader.h".

Eventually I'd like to incorporate this into the tundra build.

=end

CombinedBaseH = <<-END
#pragma once

#include "../../Render/nuRenderGLDX_Defs.h"

class nuGLDXProg_NAME : public nuGLDXProg
{
public:
	nuGLDXProg_NAME();
	virtual void		Reset();
	virtual const char*	VertSrc();
	virtual const char*	FragSrc();
	virtual const char*	Name();
	virtual bool		LoadVariablePositions();	// Performs glGet[Uniform|Attrib]Location for all variables. Returns true if all variables are found.
	virtual uint32		PlatformMask();				// Combination of nuPlatform bits.

END

CombinedBaseCpp = <<-END
#include "pch.h"
#include "NAMEShader.h"

nuGLDXProg_NAME::nuGLDXProg_NAME()
{
	Reset();
}

void nuGLDXProg_NAME::Reset()
{
	ResetBase();
RESET
}

const char* nuGLDXProg_NAME::VertSrc()
{
	return
VERT_SRC;
}

const char* nuGLDXProg_NAME::FragSrc()
{
	return
FRAG_SRC;
}

const char* nuGLDXProg_NAME::Name()
{
	return "NAME";
}


bool nuGLDXProg_NAME::LoadVariablePositions()
{
	int nfail = 0;

LOAD_FUNC_BODY
	return nfail == 0;
}

uint32 nuGLDXProg_NAME::PlatformMask()
{
	return PLATFORM_MASK;
}

END

# nature:	uniform, attribute
# type:		vec2, vec3, vec4, mat2, mat3, mat4
class Variable
	attr_accessor :nature, :type, :name
	def initialize(_nature, _type, _name)
		@nature = _nature
		@type = _type
		@name = _name
	end
end

def name_from_shader_source(source, include_vert_or_frag = true)
	if source =~ /\/(\w+)_Vert\..lsl/
		return $1 + (include_vert_or_frag ? "_" + "Vert" : "")
	end
	if source =~ /\/(\w+)_Frag\..lsl/
		return $1 + (include_vert_or_frag ? "_" + "Frag" : "")
	end
	raise "Do not understand how to name the shader #{source}"
end

def ext2name(ext)
	return ext == "hlsl" ? "DX" : "GL"
end

def escape_txt(txt)
	cpp = ""
	txt.each_line { |line|
		# The initial tab tends to preserve column formatting better
		cpp << "\"\t" + line.rstrip.gsub("\"", "\\\"") + "\\n\"\n"
	}
	return cpp
end

def escape_file(file)
	return escape_txt( File.open(file) { |f| f.read } )
end

def gen_combined(ext, vert, frag, name, filename_base)
	variables = []
	platforms = {}
	vert_src = File.open(vert) { |f| f.read }
	frag_src = File.open(frag) { |f| f.read }

	replaced = [vert_src, frag_src].collect { |src|
		cleaned_src = ""
		src.each_line { |line|
			use_line = true
			# uniform mat4 mvproj;
			if line =~ /uniform\s+(\w+)\s+(\w+);/
				variables << Variable.new("uniform", $1, $2)
			elsif line =~ /attribute\s+(\w+)\s+(\w+);/
				variables << Variable.new("attribute", $1, $2)
			end

			if line =~ /#NU_PLATFORM_(\w+)/
				use_line = false
				platform = $1
				case platform
				when "WIN_DESKTOP" then platforms[:nuPlatform_WinDesktop] = 1
				when "ANDROID" then platforms[:nuPlatform_Android] = 1
				else raise "Unrecognized platform #{platform}"
				end
			end

			cleaned_src << line if use_line
		}
		cleaned_src
	}
	vert_src, frag_src = replaced[0], replaced[1]

	platforms[:nuPlatform_All] = 1 if platforms.length == 0

	File.open(filename_base + ".h", "w") { |file|
		txt = CombinedBaseH + ""
		txt.gsub!("NAMEUC", name.upcase)
		txt.gsub!("NAME", name)
		txt.gsub!("GLDX", ext2name(ext))
		variables.each { |var|
			txt << "\tGLint v_#{var.name}; #{' ' * (15 - var.name.length)} // #{var.nature} #{var.type}\n"
		}
		txt << "};\n"
		file << txt
		file << "\n"
	}

	File.open(filename_base + ".cpp", "w") { |file|
		txt = CombinedBaseCpp + ""
		txt.gsub!("NAMEUC", name.upcase)
		txt.gsub!("NAME", name)
		txt.gsub!("GLDX", ext2name(ext))
		txt.gsub!("VERT_SRC", escape_txt(vert_src))
		txt.gsub!("FRAG_SRC", escape_txt(frag_src))
		load_func_body = ""
		reset = ""
		variables.each { |var|
			reset << "\tv_#{var.name} = -1;\n"
			if var.nature == "uniform"
				load_func_body << "\tnfail += (v_#{var.name} = glGetUniformLocation( Prog, \"#{var.name}\" )) == -1;\n"
			elsif var.nature == "attribute"
				load_func_body << "\tnfail += (v_#{var.name} = glGetAttribLocation( Prog, \"#{var.name}\" )) == -1;\n"
			else
				raise "Unrecognized variable type #{var.nature}"
			end
		}
		load_func_body << "\tif ( nfail != 0 )\n"
		load_func_body << "\t\tNUTRACE( \"Failed to bind %d variables of shader #{name}\\n\", nfail );\n"
		reset.rstrip!
		txt.gsub!("RESET", reset)
		txt.gsub!("LOAD_FUNC_BODY", load_func_body)
		txt.gsub!("PLATFORM_MASK", platforms.keys.join(" | "))
		file << txt
		file << "\n"
	}
end

# vert and frag are paths to .hlsl/.glsl files
def gen_pair(base_dir, ext, vert, frag)
	name = name_from_shader_source(vert, false)
	gen_combined(ext, vert, frag, name, base_dir + "/Processed_#{ext}/#{name}Shader")
end

def run_dir(base_dir, ext)
	shaders = Dir.glob("#{base_dir}/*.#{ext}")
	#print(shaders)

	# At present we expect pairs of "xyzFrag.glsl" and "xyzVert.glsl" (or .hlsl)
	# Maybe someday we could pair them up differently.. but not THIS DAY
	shaders.each { |candidate|
		if candidate =~ /_Vert\.#{ext}/
			vert = candidate
			frag = candidate.sub("Vert.#{ext}", "Frag.#{ext}")
			#print(vert + " " + frag + "\n")
			gen_pair(base_dir, ext, vert, frag)
		end
	}
end

run_dir("nudom/Shaders", "glsl")
run_dir("nudom/Shaders", "hlsl")