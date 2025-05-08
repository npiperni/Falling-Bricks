$emsdkPath = "D:\emscripten\emsdk" 
. "$emsdkPath\emsdk_env.ps1"

# Set the source directory and output directory
$sourceFiles = Get-ChildItem -Path "source" -Filter "*.c" | ForEach-Object { $_.FullName }
$outputDir = "build\index.html"

# Run the emcc command to compile to WebAssembly
emcc $sourceFiles -o $outputDir `
    -s USE_SDL=2 `
    -s USE_SDL_TTF=2 `
    -s USE_SDL_MIXER=2 `
    -s SDL2_MIXER_FORMATS='["mp3", "wav"]' `
    -s ALLOW_MEMORY_GROWTH=1 `
    -s ASSERTIONS=1 `
    --preload-file assets `
    -Iinclude `
    -O2 `
    -Wno-incompatible-pointer-types