
from distutils.core import setup
import py2exe

import sys
import os
sys.setrecursionlimit(5000)

manifest='''<?xml version='1.0' encoding='UTF-8' standalone='yes'?>
<assembly xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0'>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level='asInvoker' uiAccess='false' />
      </requestedPrivileges>
    </security>
  </trustInfo>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
     type='win32'
     name='Microsoft.VC90.CRT'
     version='9.0.21022.8'
     processorArchitecture='*'
     publicKeyToken='1fc8b3b9a1e18e3b' />
    </dependentAssembly>
  </dependency>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
         type="win32"
         name="Microsoft.Windows.Common-Controls"
         version="6.0.0.0"
         processorArchitecture="*"
         publicKeyToken="6595b64144ccf1df"
         language="*" />
    </dependentAssembly>
  </dependency>
</assembly>
'''

# Remove the build folder, a bit slower but ensures that build contains the latest
import shutil
shutil.rmtree("build", ignore_errors=True)
shutil.rmtree("dist", ignore_errors=True)

# my setup.py is based on one generated with gui2exe, so data_files is done a bit differently
includes = ['numpy','scipy','psutil',
    r'scipy.sparse.csgraph._validation',
    r'scipy.special._ufuncs_cxx']
excludes = [ ]
packages = []
dll_excludes = ["MSVCR80.dll","MSVCP80.dll",
    "IPHLPAPI.DLL", "NSI.dll",  "WINNSI.DLL",  "WTSAPI32.dll"]
icon_resources = []
bitmap_resources = []
other_resources = []


# add the mpl mpl-data folder and rc file
import matplotlib as mpl
data_files = mpl.get_py2exe_datafiles()

options = {"py2exe":  
            {   "compressed": 2,  
                "optimize": 1,  
                "includes": includes,  
                "excludes": excludes,
                "packages": packages,
                "dll_excludes": dll_excludes,
                "bundle_files": 1,
                "dist_dir": 'dist',
                "xref": False,
                "skip_archive": False,
                "ascii": False,
                "custom_boot_script": '',
            }  
          }  
setup(     
    version = "1.0.0",  
    description = "AccessPointCenter",  
    name = "APC",  
    options = options,  
    zipfile=None,  
    data_files=data_files,

    windows=[{
               'script':"AccessPointCenter.py",
               'other_resources' : [(24, 1, manifest)]
               }]
    
    )  

shutil.rmtree("build", ignore_errors=True)

