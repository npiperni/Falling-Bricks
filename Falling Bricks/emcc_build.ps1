# Check if running in GitHub Actions
$inCI = $env:GITHUB_ACTIONS -eq "true"

if ($inCI) {
    Write-Host "Running in GitHub Actions environment..."
} else {
    Write-Host "Running locally..."

    # Set the local path for emsdk
    $emsdkPath = "D:\emscripten\emsdk" 
    . "$emsdkPath\emsdk_env.ps1"
}

# Define directories
$sourceFiles = Get-ChildItem -Path "source" -Filter "*.c" | ForEach-Object { $_.FullName }
$outputDir = "emcc_build"
$outputFile = "$outputDir\index.html"

# Create the output directory if it doesn't exist
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir
}

# Run the emcc command to compile to WebAssembly
emcc $sourceFiles -o $outputFile `
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