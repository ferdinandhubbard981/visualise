This program can convert a char, short, int, long, long long, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, float, double, long double from binary to decimal and from decimal to binary.
It can take input in both the format ./visualise type data and ./visualise {type\;type\;...} data data ...
The main functions are:
splitArgs - splits Input into types and data (string) arrays
decodeType - converts string array input into datatype array (also checks it)
checkData - checks data validity
convertData - converts data from decimal to binary and vice-versa.
getOutput - executes functions above
