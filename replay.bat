@echo off

set CHROME=out\Release_Win\chrome

set MONITORED_FILES= ( ^
    event_recorder ^
    replay_frame ^
    forensic_replay_engine ^
    resource_data_store ^
)

del %1*

set vmodule_argument=--vmodule=

for %%n in %MONITORED_FILES% do (
    echo %%n
    set vmodule_argument=%vmodule_argument%%%n=7
)

%CHROME% --enable-logging=stderr ^
    %vmodule_argument% ^
    --profile="Profile 1" ^
    --incognito ^
    --no-sandbox ^
    --remote-debugging-port=9222 ^
    --process-per-tab ^
    --forensic-log-file=%1 ^
    about:blank