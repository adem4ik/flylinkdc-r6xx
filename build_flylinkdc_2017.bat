rem call build_get_visual_c_version.bat
del .\compiled\FlylinkDC.exe
del .\compiled\FlylinkDC.pdb
call UpdateRevision.bat %1 %2 %3 %4
if errorlevel 1 goto :error

call tools\ExtractVersion.bat %1 %2 %3 %4
if errorlevel 1 goto :error

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
chcp 437
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" FlylinkDC_2017.sln /m /t:Rebuild /p:COnfiguration="Release" /p:Platform="Win32" /flp:filename=log-x86.txt

if not exist .\compiled\FlylinkDC.exe goto :builderror

"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\signtool.exe" sign /v /d "FlylinkDC++ r6xx" /du "http://flylinkdc.blogspot.com" /fd sha1 /t http://timestamp.verisign.com/scripts/timstamp.dll ".\compiled\FlylinkDC.exe"
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\signtool.exe" sign /as /v /d "FlylinkDC++ r6xx" /du "http://flylinkdc.blogspot.com" /fd sha256 /tr http://timestamp.comodoca.com/rfc3161 /td sha256 ".\compiled\FlylinkDC.exe"

call src_gen_filename.bat -x86
7z.exe a -r -t7z -m0=lzma -mx=9 -mfb=512 -md=1024m -ms=on -x@src_exclude_hard.txt -ir@src_include_bin_x86.txt %FILE_NAME%.7z compiled/FlylinkDC.exe 

call src_gen_filename.bat -src
7z.exe a -r -t7z -m0=lzma -mx=9 -mfb=512 -md=1024m -ms=on %FILE_NAME%.7z -i@src_include.txt -x@src_exclude.txt -x!./setup/*

goto :end

:error
echo Can't update/extract version/revision
pause
goto :end

:builderror
echo Compilation error. Building terminated.
pause

:end
