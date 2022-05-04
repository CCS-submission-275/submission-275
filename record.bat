@echo off

set CHROME=out\Release_Win\chrome

set MONITORED_FILES= ( ^
    forensic_file_logger ^
)

del %2*

set vmodule_argument=--vmodule=

for %%n in %MONITORED_FILES% do (
    echo %%n
    set vmodule_argument=%vmodule_argument%%%n=7
)

set cmd=%CHROME% --enable-logging=stderr ^
    %vmodule_argument% ^
    --profile="Profile 1" ^
    --incognito ^
    --no-sandbox ^
    --remote-debugging-port=9222 ^
    --forensic-log-file=%2 ^
    %1

echo %cmd%

%cmd%