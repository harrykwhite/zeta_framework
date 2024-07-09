@echo off

set clean_dirs[0]=zeta_framework
set clean_dirs[1]=zfw_asset_packer
set clean_dirs[2]=zfw_common

setlocal enabledelayedexpansion

for /L %%i in (0, 1, 2) do (
    if exist "!clean_dirs[%%i]!\bin" (
        rd /s /q "!clean_dirs[%%i]!\bin"
    )

    if exist "!clean_dirs[%%i]!\obj" (
        rd /s /q "!clean_dirs[%%i]!\obj"
    )
)
