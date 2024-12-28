@setlocal

if "%QTOOLS%"=="" (
    set QTOOLS=C:\DEV\qp\qtools
)
python %QTOOLS%/qview/qview.py dpp.py

@endlocal
