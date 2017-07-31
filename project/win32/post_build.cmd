set ProjectDir=%1%
set ProjectName=%2%
set Configuration=%3%

set Out=%ProjectDir%..\..\out\%Configuration%
xcopy %ProjectDir%..\..\src\engine_api\*.h %Out%\include\ /s /y
xcopy %ProjectDir%\%Configuration%\*.lib %Out%\lib\ /s /y
xcopy %ProjectDir%\%Configuration%\%ProjectName%.pdb %Out%\lib\ /s /y
xcopy %ProjectDir%\%Configuration%\*.dll %Out%\lib\ /s /y

xcopy %ProjectDir%\%Configuration%\*.dll %ProjectDir%\out\%Configuration%\ /s /y

