set ProjectDir=%1%
set SolutionDir=%2%
set Configuration=%3%

set SRC=%ProjectDir%..\..\..\src
set Include=%ProjectDir%..\..\..\out\%Configuration%\include
set Lib=%ProjectDir%..\..\..\out\%Configuration%\lib\
set Bin=%ProjectDir%..\..\..\out\%Configuration%\bin\
set Pdb=%ProjectDir%..\..\..\out\%Configuration%\pdb\
::set Test=%ProjectDir%..\..\..\test\out\%Configuration%\

xcopy %SRC%\engine_api\*.h %Include%\ /s /y
xcopy %SolutionDir%\%Configuration%\*.lib %Lib% /s /y
xcopy %SolutionDir%\%Configuration%\*.pdb %Pdb% /s /y
xcopy %SolutionDir%\%Configuration%\*.dll %Bin% /s /y
::xcopy %SolutionDir%\%Configuration%\*.dll %Test% /s /y

