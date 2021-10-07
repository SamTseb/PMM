copy /y NUL test.txt
@ < nul set /p str= ,>> test.txt
for /L %%P IN (1,1,10) DO @ < nul set /p str=%%P,>> test.txt
@echo. >> test.txt
for /L %%S IN (100,100,300) DO (
@ < nul set /p str=%%S,>> test.txt
for /L %%P IN (1,1,10) DO (
mpiexec -n %%P Arch_Lab_1.exe %%S >> test.txt
@start /wait "" mpiexec.exe
@ < nul set /p str="," >> test.txt
)
@echo. >> test.txt
)

pause