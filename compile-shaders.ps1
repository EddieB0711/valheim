function get-shaderFiles {
    $glslFileTypes = "*.vert", "*.frag"
    $excludeDirectories = "vendors"

    get-childitem -path $PSScriptRoot -recurse -include $glslFileTypes -file | where { $_.FullName -notmatch $excludeDirectories }
}

function compile-shaderFiles {
    param (
        $shaderFiles
    )

    foreach ($file in $shaderFiles) {
        & $glsl 
    }
}

$shaderFiles = get-shaderFiles
compile-shaderFiles $shaderFiles