# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2020 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#

import os

#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')

# Create a build environment for the ARM9 based netX chips.
#env_arm9 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc'])
#env_arm9.CreateCompilerEnv('NETX500', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX56', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX50', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX10', ['arch=armv5te'])

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
#env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
#env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])
#env_cortexM4.CreateCompilerEnv('NETX90_APP', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('#targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
# This is the list of sources. The elements must be separated with whitespace
# (i.e. spaces, tabs, newlines). The amount of whitespace does not matter.
sources = """
	src/header.c
	src/init_muhkuh.S
	src/main.c
"""

#----------------------------------------------------------------------------
#
# Build all files.
#

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '#targets/version', 'targets/netx90_module_hispi/include']

# This is the demo for the COM side.
tEnvCom = atEnv.NETX90.Clone()
tEnvCom.Append(CPPPATH = astrIncludePaths)
tEnvCom.Replace(LDFILE = 'src/netx90/netx90_com_intram.ld')
tSrcCom = tEnvCom.SetBuildPath('targets/netx90', 'src', sources)
tElfCom = tEnvCom.Elf('targets/netx90_bluetooth_id.elf', tSrcCom + tEnvCom['PLATFORM_LIBRARY'])
tTxtCom = tEnvCom.ObjDump('targets/netx90_bluetooth_id.txt', tElfCom, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
BLUETOOTH_ID_NETX90_COM = tEnvCom.ObjCopy('targets/netx90_bluetooth_id.bin', tElfCom)


#----------------------------------------------------------------------------
#
# Build the documentation.
#
# Get the default attributes.
aAttribs = atEnv.DEFAULT['ASCIIDOC_ATTRIBUTES']
# Add some custom attributes.
aAttribs.update(dict({
    # Use ASCIIMath formulas.
    'asciimath': True,

    # Embed images into the HTML file as data URIs.
    'data-uri': True,

    # Use icons instead of text for markers and callouts.
    'icons': True,

    # Use numbers in the table of contents.
    'numbered': True,

    # Generate a scrollable table of contents on the left of the text.
    'toc2': True,

    # Use 4 levels in the table of contents.
    'toclevels': 4
}))

#doc = atEnv.DEFAULT.Asciidoc('targets/doc/netx90_app_bridge.html', 'doc/netx90_app_bridge.asciidoc', ASCIIDOC_BACKEND='html5', ASCIIDOC_ATTRIBUTES=aAttribs)


# ---------------------------------------------------------------------------
#
# Install the files to the testbench.
#
atFiles = {
    'targets/testbench/netx/netx90_bluetooth_id.bin':      BLUETOOTH_ID_NETX90_COM
}
for tDst, tSrc in atFiles.iteritems():
    Command(tDst, tSrc, Copy("$TARGET", "$SOURCE"))
