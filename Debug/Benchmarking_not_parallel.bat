copy /y NUL test_not_parallel.txt
@ < nul set /p str= ,>> test_not_parallel.txt
for /L %%P IN (1,1,1) DO @ < nul set /p str=%%P,>> test_not_parallel.txt
@echo. >> test_not_parallel.txt
for /L %%S IN (100,100,20000) DO (
@ < nul set /p str=%%S,>> test_not_parallel.txt
for /L %%P IN (1,1,1) DO (
mpiexec -n %%P Arch_Lab_1_not_parallel.exe %%S >> test_not_parallel.txt
@start /wait "" mpiexec.exe
@ < nul set /p str="," >> test_not_parallel.txt
)
@echo. >> test_not_parallel.txt
)

pause