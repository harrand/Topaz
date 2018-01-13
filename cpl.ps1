$script_directory = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$source_directory = "$script_directory\src"
$includes_directory = "$script_directory\inc"
$libraries_directory = "$script_directory\lib"
$runtime_dependencies_directory = "$script_directory\res\dep"
$compilation_directory = "$script_directory\cpl"
$date = Get-Date -UFormat "%Y\%m\%d"
$link_directory = "$compilation_directory\$date"
$binaries_directory = "$link_directory\bin"

write-host -foreground white "Compilation parameters for " -nonewline
write-host -foreground yellow "Topaz:"

write-host "`tscript_directory = `"$script_directory`""
write-host "`tsource_directory = `"$source_directory`""
write-host "`tincludes_directory = `"$includes_directory`""
write-host "`tlibraries_directory = `"$libraries_directory`""
write-host "`truntime_dependencies_directory = `"$runtime_dependencies_directory`""
write-host "`tlink_directory = `"$link_directory`""
write-host "`tbinaries_directory = `"$binaries_directory`""

Push-Location -Path $source_directory

write-host "Compiling all " -nonewline
write-host -foreground yellow "C" -nonewline
write-host " files" -nonewline
write-host -foreground yellow "(*.c):"

gcc -c *.c -w -I $includes_directory

write-host "Compiling all " -nonewline
write-host -foreground yellow "C++" -nonewline
write-host " files" -nonewline
write-host -foreground yellow "(*.cpp):"

Get-ChildItem $source_directory | Where {$_.extension -like ".cpp"} | ForEach-Object {
	if($_ -like "test.cpp") {
		write-host -foreground red "`tSkipping $_..."
	}
	else {
		write-host "`tCompiling " -nonewline
		write-host -foreground yellow "$_..."
		g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c $_ -I $includes_directory
	}
}

write-host "Moving all compiled " -nonewline
write-host -foreground yellow "object (.o) " -nonewline
write-host "files to link_directory..."

Get-ChildItem -Path $source_directory -Filter "*.o" -Recurse | ForEach-Object {
	write-host "Moving " -nonewline
	write-host -foreground yellow "$_..."
	Move-Item $_ -Destination $link_directory
}
Push-Location -Path $link_directory
write-host "Linking to " -nonewline
write-host -foreground yellow "libtopazdll.a " -nonewline
write-host "and " -nonewline
write-host -foreground yellow "topaz.dll..."
&"g++" "-std=c++17" "-Wall" "-pedantic-errors" "-O3" "-shared" "-Wl,-no-undefined,--enable-runtime-pseudo-reloc,--out-implib,libtopazdll.a" "*.o" "-L" "$libraries_directory" "-lOpenGL32" "-lSDL2" "-lSDL2_mixer" "-lSDL2_ttf" "-lSDL2main" "-lmdl" "-o" "topaz.dll"
write-host "Moving outputs " -nonewline
write-host -foreground yellow "libtopazdll.a " -nonewline
write-host "and " -nonewline
write-host -foreground yellow "topaz.dll..." -nonewline
write-host " to binaries_directory..."
Move-Item topaz.dll $binaries_directory
Move-Item libtopazdll.a $binaries_directory
Pop-Location #Now back in source directory
write-host "Now compiling " -nonewline
write-host -foreground yellow "test.cpp " -nonewline
write-host "by itself..."
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c test.cpp -I $includes_directory
write-host "Moving " -nonewline
write-host -foreground yellow "test.o " -nonewline
write-host " to binaries_directory with the outputs..."

Move-Item test.o $binaries_directory

write-host "Copying all dependencies from " -nonewline
write-host -foreground yellow "runtime_dependencies_directory " -nonewline
write-host "to binaries_directory..."
Set-Location $runtime_dependencies_directory

Get-ChildItem -Path $runtime_dependencies_directory -Recurse | ForEach-Object {
	write-host "Copying " -nonewline
	write-host -foreground yellow "$_..."
	Copy-Item $_ -Destination $binaries_directory
}

write-host "Creating the final executable " -nonewline
write-host -foreground yellow "topaz_test.exe..."
Set-Location $binaries_directory
g++ -O3 -o topaz_test.exe test.o $script_directory\res\exe\topaz_test.res -static-libstdc++ -L. -L$libraries_directory -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2main -lmdl -ltopaz
write-host -foreground blue "Compilation complete."

Pop-Location