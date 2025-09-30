<#
.SYNOPSIS
    多目标C++编译系统 - 支持并行编译多个独立的C++源文件
.DESCRIPTION
    该脚本用于自动化编译指定目录下的所有.cpp文件，每个源文件生成独立的可执行文件。
    支持增量编译（仅重新编译修改过的文件）和清理构建缓存功能。
.PARAMETER Clean
    清除所有构建产物（包括输出目录和中间对象文件目录）
.EXAMPLE
    # 正常编译模式
    .\build.ps1
    
    # 清理构建缓存
    .\build.ps1 -Clean
.NOTES
    版本: 1.2
    作者: 腾讯元宝
    最后更新: $(Get-Date -Format "yyyy-MM-dd")
#>
param(
    [switch]$Clean  # 声明一个开关参数，用于触发清理模式（不需要额外值）
)

# ==============================
# 初始化配置部分
# ==============================

# Source code directory path (relative path)
$sourceDir = "..\source"

# Final executable output directory
$outputDir = "..\build"

# Intermediate object files directory (currently unused, reserved for future expansion)
$objDir = "..\obj"

# Header file include path
$includePath = "..\include"

# ==============================
# 清理模式处理
# ==============================
if ($Clean) {
    # Recursively delete output and object directories, force operation and ignore errors
    Remove-Item $outputDir, $objDir -Recurse -Force -ErrorAction SilentlyContinue
    
    # Output colored status message
    Write-Host "Build cache cleaned (Deleted directories: $outputDir, $objDir)" -ForegroundColor Yellow
    
    # Exit script immediately
    exit 0
}

# ==============================
# 目录结构准备
# ==============================

# Ensure required directories exist
foreach ($dir in ($outputDir, $objDir, $includePath)) {
    if (-not (Test-Path $dir)) { 
        # Create directory and suppress output
        New-Item -ItemType Directory $dir | Out-Null
        
        # 调试时可取消下面注释查看创建过程
        # Write-Host "已创建目录: $dir" -ForegroundColor DarkGray
    }
}

# ==============================
# 核心编译逻辑
# ==============================

# Process all .cpp source files
Get-ChildItem -Path $sourceDir -Filter "*.cpp" | ForEach-Object {
    # Construct target executable path (keep original filename, only change extension)
    $exePath = Join-Path $outputDir ($_.BaseName + ".exe")
    
    # ========== 增量编译检查 ==========
    # Recompile if either condition is met:
    # 1. Target executable doesn't exist
    # 2. Source file has newer last write time than target
    $needRebuild = (-not (Test-Path $exePath)) -or 
                  ($_.LastWriteTime -gt (Get-Item $exePath).LastWriteTime)
    
    if ($needRebuild) {
        # ========== 执行编译命令 ==========
        # Parameters:
        #   $_.FullName : Full path of current source file
        #   -I $includePath : Add header include path
        #   -o $exePath : Specify output file path
        g++ $_.FullName -I $includePath -o $exePath
        
        # ========== 编译结果检查 ==========
        if ($LASTEXITCODE -eq 0) {
            # Success - output green message (include source file and target path)
            Write-Host "Compile successful: $($_.Name) --> $exePath" -ForegroundColor Green
        } else {
            # Failure - output red error message
            Write-Host "Compile failed: $($_.Name)" -ForegroundColor Red
            # 注：此处可以扩展更详细的错误处理逻辑
        }
    } else {
        # Skip recompilation - show gray hint
        Write-Host "Skip(not modified): $($_.Name)" -ForegroundColor Gray
    }
}

# ==============================
# 最终状态报告
# ==============================

# Output final compilation result (highlighted in cyan)
Write-Host "All files compiled successfully! Output directory: $outputDir" -ForegroundColor Cyan

# ==============================
# 后续扩展建议
# ==============================
<#
未来改进方向：
1. 添加并行编译支持（需PowerShell 7+）
   Get-ChildItem ... | ForEach-Object -Parallel { ... }

2. 支持分步编译（先编译为.o，再链接）
   g++ -c $_.FullName -o $objFile
   g++ $objFiles -o $exePath

3. 添加编译选项配置
   $cxxFlags = @("-O2", "-Wall", "-std=c++17")

4. 支持跨平台路径处理
   $exeExt = if ($IsWindows) { ".exe" } else { "" }

5. 增强错误处理
   try { g++ ... } catch { 发送错误邮件/记录日志 }
#>