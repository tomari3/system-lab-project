clear
cd ..
echo "Compiling Assembler...\n"
make
echo "\nRunning Assembler...\n"
echo "Output:\n"
./assembler test/input1 
make clean

