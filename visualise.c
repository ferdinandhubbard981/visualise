#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

const char* ERRORMESSAGE = "Input error.";
enum {NUMOFDATATYPES = 13};
const int DENARYBASE = 10;
const int ZERO = 0;
const int NINE = 9;
const char ZEROCHAR = '0'; //no magic numbers rule
const char ONECHAR = '1';
const int NIBBLE = 4;
enum Datatype {CHAR, SHORT, INT, LONG, LONGLONG, UCHAR, USHORT, UINT, ULONG, ULONGLONG, FLOAT, DOUBLE, LONGDOUBLE};
static const int NUMOFNIBBLES[NUMOFDATATYPES] = {2*sizeof(char), 2*sizeof(short), 2*sizeof(int), 2*sizeof(long), 2*sizeof(long long), 2*sizeof(unsigned char), 2*sizeof(unsigned short), 2*sizeof(unsigned int), 2*sizeof(unsigned long), 2*sizeof(unsigned long long), 2*sizeof(float), 2*sizeof(double), 2*sizeof(long double)};
static const char* DATATYPESTRINGS[NUMOFDATATYPES] = {"char", "short", "int", "long", "long long", "unsigned char", "unsigned short", "unsigned int", "unsigned long", "unsigned long long", "float", "double", "long double"};
typedef enum Datatype datatype;

struct char2d {
  char** arr;
  int len;
  int* sublen;
};
typedef struct char2d char2d;

struct datatype1d {
  datatype* arr;
  int len;
};
typedef struct datatype1d datatype1d;

//MISCELLANEOUS FUNCTIONS
bool isDigit(char c) {
  int val = c - ZEROCHAR;
  return (val >= ZERO && val <= NINE);
}

void removeChar(char* arr, int index) {
  for (int i = index; i < strlen(arr); i++) { //+1 because strlen doesn't include string terminating character(\0)
    *(arr+i) = *(arr+i+1);
  }
}

void destroyChar2d(char2d* foo) {
  for(int i = ZERO; i < foo->len; i++) {
    free(*(foo->arr + i));
  }
  free(foo->arr);
  free(foo->sublen);
}

void removeTrailingZeros(char* decimal) {
  //find index of dot
  bool foundDot = false;
  int dotIndex = ZERO;
  for (int i = ZERO; i < strlen(decimal); i++) {
    if (decimal[i] == '.') {
      foundDot = true;
      dotIndex = i;
    }
  }
  //remove every '0' starting from the end of the number until the dot whilst a non-zero number hasn't occured
  if (foundDot) {
    int index = strlen(decimal) - 1;
    bool foundNonZero = false;
    while (!foundNonZero && index > dotIndex) {
      if (decimal[index] == ZEROCHAR) removeChar(decimal, index);
      else foundNonZero = true;
      index--;
    }
  }
}
//splitArgs
int getnumOfDatatypeArgs(int n, char* args[n]) {

  int total = ZERO;
  while (total < n-1 && !isDigit(args[total+1][0]) && !(args[total+1][0] == '-')) {
    total++;
  }
  return total;
}

void splitArgs(char* args[], int n, char2d* datatypeArgs, char2d* dataArgs) {
  //Datatype array
  datatypeArgs->len = getnumOfDatatypeArgs(n, args);
  datatypeArgs->sublen = (int*)malloc(datatypeArgs->len * sizeof(int));
  datatypeArgs->arr = (char**)malloc(datatypeArgs->len * sizeof(char*));
  int datatypeIndex = 1;
  for (int i = ZERO; i < datatypeArgs->len; i++) {
    *(datatypeArgs->arr + i) = (char*)malloc((strlen(args[i + datatypeIndex]) + 1) * sizeof(char)); //to accomodate destoy function
    strcpy(*(datatypeArgs->arr + i), args[i + datatypeIndex]);
    //*(datatypeArgs->arr + i) = args[i + datatypeIndex];
    *(datatypeArgs->sublen + i) = strlen(args[i + datatypeIndex]);
  }

  //data array
  dataArgs->len = n - 1 - datatypeArgs->len;
  dataArgs->sublen = (int*)malloc(dataArgs->len * sizeof(int));
  int dataIndex = datatypeArgs->len + 1;
  dataArgs->arr = (char**)malloc(dataArgs->len * sizeof(char*));
  for (int i = ZERO; i < dataArgs->len; i++) {
    *(dataArgs->arr + i) = (char*)malloc((strlen(args[i + dataIndex]) + 1) * sizeof(char)); //to accomodate destoy function
    strcpy(*(dataArgs->arr + i), args[i + dataIndex]);
    //*(dataArgs->arr + i) = args[i + dataIndex];
    *(dataArgs->sublen + i) = strlen(args[i + dataIndex]);
  }

}

//DECODE TYPE
bool convertToDatatype(char* input, datatype1d* types) {
  for(int i = ZERO; i < NUMOFDATATYPES; i++) {
    if (strcmp(input, DATATYPESTRINGS[i]) == ZERO) {
      types->len++;
      types->arr = realloc(types->arr, types->len * sizeof(datatype));

      *(types->arr+types->len-1) = i;
      return true;
    }
  }
  return false;
}

bool decodeType(char2d* datatypeArgs, datatype1d* types) { //datatypeArgs is type input
  const char* delimiter = ";";
  types->arr = (datatype*)malloc(0);
  types->len = ZERO;
  bool output = true;
  //getting length of result string when concatenating all type args
  int len = ZERO;
  for (int i = ZERO; i < datatypeArgs->len; i++) {
    len += *(datatypeArgs->sublen+i);
    len += 1;
  }
  //concatenating separated type input (because of how args[] separates the input) into what user typed in
  char* typeString = (char*)malloc(len*sizeof(char));
  *typeString = ZERO;
  for(int i = ZERO; i < datatypeArgs->len; i++) {
    strcat(typeString, *(datatypeArgs->arr+i));
    if (i != datatypeArgs->len - 1) strcat(typeString, " "); //we don't want to add a space after the last substring
                                                            //because if input was "long long", then we would get "long long ";
  }
  //isStruct? split into string array containing each datatype
  if (*typeString == '{' && *(typeString + len-2) == '}') { //iff true then isStruct is true
    //remove brackets (first and last characters)
    removeChar(typeString, 0);
    removeChar(typeString, strlen(typeString)-1);
    //split string at every occurence of "\;" into substrings, converting each substring to datatype and adding that to types array
    char* substring = strtok(typeString, delimiter);
    while (substring != NULL && output) {
      if (!convertToDatatype(substring, types)) output = false; //converting to datatype and adding to types array
      substring = strtok(NULL, delimiter);
    }
  }
  else {
    if (!convertToDatatype(typeString, types)) output = false; //if only 1 datatype was entered: only one datatype is added to the list
  }
  free(typeString);
  return output;
}

//CHECK DATA
//OVERFLOW and UNDERFLOW checking
bool checkLongDouble(char* decimalString) {
  strtold(decimalString, NULL);
  bool output = true;
  if (errno == ERANGE) {
    output = false;
    errno = ZERO;
  }
  return output;
}

bool checkDouble(char* decimalString) {
  strtod(decimalString, NULL);
  bool output = true;
  if (errno == ERANGE) {
    output = false;
    errno = ZERO;
  }
  return output;
}

bool checkFloat(char* decimalString) {
  strtof(decimalString, NULL);
  bool output = true;
  if (errno == ERANGE) {
    output = false;
    errno = ZERO;
  }
  return output;
}
bool checkFloatingPointInRange(char* decimalString, datatype type) {

  bool output = true;
  if (type == LONGDOUBLE) output = checkLongDouble(decimalString);
  if (type == DOUBLE) output = checkDouble(decimalString);
  if (type == FLOAT) output = checkFloat(decimalString);
  return output;
}

bool checkUnsignedInRange(char* decimalString, datatype type) {
  unsigned long long number = strtoull(decimalString, NULL, 10);
  bool output = true;
  if (errno == ERANGE) {
    output = false;
    errno = ZERO;
  }
  if (type == ULONG && number > ULONG_MAX) output = false;
  if (type == UINT && number > UINT_MAX) output = false;
  if (type == USHORT && number > USHRT_MAX) output = false;
  if (type == UCHAR && number > UCHAR_MAX) output = false;


  return output;
}

bool checkSignedInRange(char* decimalString, datatype type) {

  long long number = strtoll(decimalString, NULL, 10);
  bool output = true;
  if (errno == ERANGE) {
    output = false;
    errno = ZERO;
  }
  if (type == LONG && (number > LONG_MAX || number < LONG_MIN)) output = false;
  if (type == INT && (number > INT_MAX || number < INT_MIN)) output = false;
  if (type == SHORT && (number > SHRT_MAX || number < SHRT_MIN)) output = false;
  if (type == CHAR && (number > CHAR_MAX || number < CHAR_MIN)) output = false;


  return output;
}

bool checkOverFlow(char* decimalString, datatype type) {
  bool output = true;
  if (type < 5 && !checkSignedInRange(decimalString, type)) output = false;
  else if (type < 10 && !checkUnsignedInRange(decimalString, type)) output = false;
  else if (type < 13 && !checkFloatingPointInRange(decimalString, type)) output = false;
  return output;
}

bool checkDecimal(char* decimalString, datatype type) {
  // TODO CHECK LEADING ZERO's
  //only digits, first character may be a '-', there may be a '.' once and not as the first or last character
  bool output = true;
  bool hasADot = false;
  bool isNegative = false;
  for (int i = ZERO ; i < strlen(decimalString); i++) {
    if (!isDigit(decimalString[i]) && !((decimalString[i] == '-' && i == ZERO) || (decimalString[i] == '.' && (!hasADot || i != ZERO || i != strlen(decimalString)-1)))) {
      output = false;
    }
    if (decimalString[i] == '.') hasADot = true;
    if (decimalString[i] == '-') isNegative = true;
  }
  //checking if decimalString has leading zero's
  if (strlen(decimalString) > 1) {
    if (isNegative) {
      if (decimalString[1] == ZEROCHAR && decimalString[2] != '.') output = false;
    }
    else {
      if (decimalString[0] == ZEROCHAR && decimalString[1] != '.') output = false;
    }
  }
  //checking that (number is decimal) => (type is float, double or long double) is true
  if (hasADot && !(type == FLOAT || type == DOUBLE || type == LONGDOUBLE)) output = false;
  //checking that (number is negative) => ~(type is unsigned) is true
  if (isNegative && (type == UCHAR || type == USHORT || type == UINT || type == ULONG || type == ULONGLONG)) output = false;

  //decimal number must be within bounds of datatype so check (OVERFLOW and UNDERFLOW)
  //OVERFLOW
  if (!checkOverFlow(decimalString, type)) output = false;
  //UNDERFLOW
  //if (!checkUnderflow()) output = false;

  return output;
}

bool checkDecimalList(char2d* decimalString, datatype1d* types) {
  bool output = true;
  for (int i = ZERO; i < decimalString->len; i++) {
    if (!checkDecimal(*(decimalString->arr + i), *(types->arr + i))) output = false;
  }
  return output;
}

bool checkBinary(char2d* binary, datatype1d* types) {
  bool output = true;
  //checking each segment of binary is of length 4
  //and that they are only '1' or '0'
  for (int i = ZERO; i < binary->len; i++) {
    if (*(binary->sublen + i) != NIBBLE) output = false;
    for (int j = ZERO; j < NIBBLE; j++) {
      if (*(*(binary->arr + i) + j) != ZEROCHAR && *(*(binary->arr + i) + j) != ONECHAR) output = false;
    }
  }
  //check that number of nibbles matches input types
  int expectedNumOfNibbles = ZERO;
  for (int i = ZERO; i < types->len; i++) {
    expectedNumOfNibbles += NUMOFNIBBLES[*(types->arr + i)];
  }
  if (binary->len != expectedNumOfNibbles) output = false;
  return output;
}

bool checkData(char2d* dataArgs, datatype1d* types) {
  bool output = true;
  if (dataArgs->len < types->len) output = false;
  else if (dataArgs->len == types->len) output = checkDecimalList(dataArgs, types);
  else output = checkBinary(dataArgs, types);
  return output;
}

//CONVERT DATA
char* convertDecimalToBinary(void* val, datatype* type)
{
  int len = CHAR_BIT * (NUMOFNIBBLES[*type] / 2);
  char* output = (char*)malloc((len+len/4 /*- 1 + 1 for \0*/) * sizeof(char));
  unsigned char *ptr = (unsigned char*) val;
  int buffer = ZERO;
  for (int index = len; index-- ;) {
    output[len - (index+1) + buffer] = (ptr[index/CHAR_BIT] & (1 << (index%CHAR_BIT))) ? ONECHAR: ZEROCHAR;
    if (index % 4 == ZERO && index != ZERO) {
      buffer++;
      output[len - (index+1) + buffer] = ' ';
      }
  }
  output[len + buffer] = ZERO;
  return output;
}

char* getCharBinary(char* valString, datatype* type) {

  char* output;
  //void* output = (void*)malloc(sizeof(long double)); //long double has greatest size
  if (*type == CHAR) {
    char val = (char)strtol(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == SHORT) {
    short val = (short)strtol(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == INT) {
    int val = (int)strtol(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == LONG) {
    long val = (long)strtol(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == LONGLONG) {
    long long val = (long long)strtoll(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == UCHAR) {
    unsigned char val = (unsigned char)strtoul(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == USHORT) {
    unsigned short val = (unsigned short)strtoul(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == UINT) {
    unsigned int val = (unsigned int)strtoul(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == ULONG) {
    unsigned long val = (unsigned long)strtoul(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == ULONGLONG) {
    unsigned long long val = (unsigned long long)strtoull(valString, NULL, DENARYBASE);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == FLOAT) {
    float val = (float)strtof(valString, NULL);
    output = convertDecimalToBinary(&val, type);
  }

  else if (*type == DOUBLE) {
    double val = (double)strtod(valString, NULL);
    output = convertDecimalToBinary(&val, type);
  }

  else {
    long double val = (long double)strtold(valString, NULL);
    output = convertDecimalToBinary(&val, type);
  }
  return output;

}
char* convertDecimalArr(char2d* dataArgs, datatype1d* types) {
  char* output = (char*)malloc(1 * sizeof(char));
  output[0] = ZERO;
  for (int i = ZERO; i < types->len; i++) {
    char* splitInput = getCharBinary(*(dataArgs->arr + i), &types->arr[i]);
    int space = (i != types->len - 1) ? 1 : 0;
    output = realloc(output, (strlen(output) + strlen(splitInput) + space+1) * sizeof(char));
    strcat(output, splitInput);
    if (space == 1) strcat(output, " ");
    free(splitInput);
  }
  return output;
}

unsigned long long getBits(const char* input) {
  unsigned long long out = ZERO;
  for (; *input; ++input)
  {
      out = (out << 1) + (*input - ZEROCHAR);
  }
  return out;
}

//BITS to decimal
char bitStringToChar(const char* p)
{
    unsigned long long x = getBits(p);
    char d;
    memcpy(&d, &x, sizeof(char));
    return d;
}

short bitStringToShort(const char* p)
{
    unsigned long long x = getBits(p);
    short d;
    memcpy(&d, &x, sizeof(short));
    return d;
}

int bitStringToInt(const char* p)
{
    unsigned long long x = getBits(p);
    int d;
    memcpy(&d, &x, sizeof(int));
    return d;
}

long bitStringToLong(const char* p)
{
    unsigned long long x = getBits(p);
    long d;
    memcpy(&d, &x, sizeof(long));
    return d;
}

long long bitStringToLonglong(const char* p)
{
    unsigned long long x = getBits(p);
    long long d;
    memcpy(&d, &x, sizeof(long long));
    return d;
}

unsigned char bitStringToUnsignedChar(const char* p)
{
    unsigned long long x = getBits(p);
    unsigned char d;
    memcpy(&d, &x, sizeof(unsigned char));
    return d;
}

unsigned short bitStringToUnsignedShort(const char* p)
{
    unsigned long long x = getBits(p);
    unsigned short d;
    memcpy(&d, &x, sizeof(unsigned short));
    return d;
}

unsigned int bitStringToUnsignedInt(const char* p)
{
    unsigned long long x = getBits(p);
    unsigned int d;
    memcpy(&d, &x, sizeof(unsigned int));
    return d;
}

unsigned long bitStringToUnsignedLong(const char* p)
{
    unsigned long long x = getBits(p);
    unsigned long d;
    memcpy(&d, &x, sizeof(unsigned long));
    return d;
}

unsigned long long bitStringToUnsignedLonglong(const char* p)
{
    unsigned long long x = getBits(p);
    unsigned long long d;
    memcpy(&d, &x, sizeof(unsigned long long));
    return d;
}

float bitStringToFloat(const char* p)
{
    unsigned long long x = getBits(p);
    float d;
    memcpy(&d, &x, sizeof(float));
    return d;
}

double bitStringToDouble(const char* p)
{
    unsigned long long x = getBits(p);
    double d;
    memcpy(&d, &x, sizeof(double));
    return d;
}

long double bitStringToLongdouble(const char* p)
{
    unsigned long long x = getBits(p);
    long double d;
    memcpy(&d, &x, sizeof(long double));
    return d;
}


//Decimal to STRING
char* charToString(char val) {
  char* output;
  int length = snprintf(NULL, 0, "%d", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%d", val);
  return output;
}

char* shortToString(short val) {
  char* output;
  short length = snprintf(NULL, 0, "%d", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%d", val);
  return output;
}

char* intToString(int val) {
  char* output;
  int length = snprintf(NULL, 0, "%d", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%d", val);
  return output;
}

char* longToString(long val) {
  char* output;
  long length = snprintf(NULL, 0, "%ld", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%ld", val);
  return output;
}

char* longlongToString(long long val) {
  char* output;
  long long length = snprintf(NULL, 0, "%lld", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%lld", val);
  return output;
}

char* unsignedCharToString(unsigned char val) {
  char* output;
  unsigned char length = snprintf(NULL, 0, "%u", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%u", val);
  return output;
}

char* unsignedShortToString(unsigned short val) {
  char* output;
  unsigned short length = snprintf(NULL, 0, "%u", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%u", val);
  return output;
}

char* unsignedIntToString(unsigned int val) {
  char* output;
  unsigned int length = snprintf(NULL, 0, "%u", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%u", val);
  return output;
}

char* unsignedLongToString(unsigned long val) {
  char* output;
  unsigned long length = snprintf(NULL, 0, "%lu", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%lu", val);
  return output;
}

char* unsignedLonglongToString(unsigned long long val) {
  char* output;
  unsigned long long length = snprintf(NULL, 0, "%llu", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%llu", val);
  return output;
}

char* floatToString(float val) {
  char* output;
  float length = snprintf(NULL, 0, "%f", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%f", val);
  return output;
}

char* doubleToString(double val) {
  char* output;
  double length = snprintf(NULL, 0, "%lf", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%lf", val);
  return output;
}

char* longDoubleToString(long double val) {
  char* output;
  long double length = snprintf(NULL, 0, "%Lf", val); //This returns length of val as a string
  output = (char*)malloc((length+1) * sizeof(char));
  snprintf(output, length+1, "%Lf", val);
  return output;
}

char* getDecimalString(char* binary, datatype* type) {
  char* output;
  if (*type == CHAR) {
    char val = bitStringToChar(binary);
    output = charToString(val);
  }

  if (*type == SHORT) {
    short val = bitStringToShort(binary);
    output = shortToString(val);
  }

  if (*type == INT) {
    int val = bitStringToInt(binary);
    output = intToString(val);
  }

  if (*type == LONG) {
    long val = bitStringToLong(binary);
    output = longToString(val);
  }

  if (*type == LONGLONG) {
    long long val = bitStringToLonglong(binary);
    output = longlongToString(val);
  }

  if (*type == UCHAR) {
    unsigned char val = bitStringToUnsignedChar(binary);
    output = unsignedCharToString(val);
  }

  if (*type == USHORT) {
    unsigned short val = bitStringToUnsignedShort(binary);
    output = unsignedShortToString(val);
  }

  if (*type == UINT) {
    unsigned int val = bitStringToUnsignedInt(binary);
    output = unsignedIntToString(val);
  }

  if (*type == ULONG) {
    unsigned long val = bitStringToUnsignedLong(binary);
    output = unsignedLongToString(val);
  }

  if (*type == ULONGLONG) {
    unsigned long long val = bitStringToUnsignedLonglong(binary);
    output = unsignedLonglongToString(val);
  }

  if (*type == FLOAT) {
    float val = bitStringToFloat(binary);
    output = floatToString(val);
    removeTrailingZeros(output);
  }

  if (*type == DOUBLE) {
    double val = bitStringToDouble(binary);
    output = doubleToString(val);
    removeTrailingZeros(output);
  }

  if (*type == LONGDOUBLE) {
    long double val = bitStringToLongdouble(binary);
    output = longDoubleToString(val);
    removeTrailingZeros(output);
  }
  return output;
}

void groupBinaryInput(char2d* output, char2d* dataArgs, datatype1d* types) {
  output->arr = (char**)malloc(types->len * sizeof(char*));
  output->len = types->len;
  output->sublen = (int*)malloc(output->len * sizeof(int));
  int currentDataArgsIndex = ZERO;
  for (int i = ZERO; i < types->len; i++) {
    *(output->arr + i) = (char*)malloc((NUMOFNIBBLES[types->arr[i]] * 4 + 1) * sizeof(char));
    *(output->sublen + i) = NUMOFNIBBLES[types->arr[i]] * 4;
    **(output->arr + i) = ZERO;
    for (int j = ZERO; j < NUMOFNIBBLES[*(types->arr + i)]; j++) {
      strcat(*(output->arr + i), *(dataArgs->arr + currentDataArgsIndex));
      currentDataArgsIndex++;
    }
    //strcat(*(dataArgs->arr + i), "";
  }
}

char* convertBinaryArr(char2d* dataArgs, datatype1d* types) {
  char2d groupedBinary;
  groupBinaryInput(&groupedBinary, dataArgs, types);
  char* output = (char*)malloc(1*sizeof(char));
  output[0] = ZERO;
  for (int i = ZERO; i < types->len; i++) {
    char* splitInput = getDecimalString(*(groupedBinary.arr + i), &types->arr[i]);
    int space = (i != types->len - 1) ? 1 : 0;
    output = realloc(output, (strlen(output) + strlen(splitInput) + space + 1) * sizeof(char));
    strcat(output, splitInput);
    if (space == 1) strcat(output, " ");
    free(splitInput);
  }
  destroyChar2d(&groupedBinary);
  return output;
}

char* convertData(char2d* dataArgs, datatype1d* types) {
  char* output;
  if (dataArgs->len == types->len) output = convertDecimalArr(dataArgs, types);
  else output = convertBinaryArr(dataArgs, types);
  return output;

}
char* getOutput(int n, char* args[n]) {
  char* output;
  bool errorFree = true;
  char2d datatypeArgs, dataArgs;
  splitArgs(args, n, &datatypeArgs, &dataArgs);
  //printArgs(datatypeArgs, dataArgs);
  datatype1d types;
  bool typeIsMalloced = errorFree;
  if(!decodeType(&datatypeArgs, &types) || types.len == ZERO) {
    output = (char*)malloc(sizeof(char) * (strlen(ERRORMESSAGE) + 1));
    strcpy(output, ERRORMESSAGE);
    errorFree = false;
  }
  destroyChar2d(&datatypeArgs);
  if (errorFree) {
    if (!checkData(&dataArgs, &types)) {
      output = (char*)malloc(sizeof(char) * (strlen(ERRORMESSAGE) + 1));
      strcpy(output, ERRORMESSAGE);
      errorFree = false;
    }
  }

  if (errorFree) {
    output = convertData(&dataArgs, &types);
    //printf("output: %s\n", output);
  }
  if (typeIsMalloced) free(types.arr);
  destroyChar2d(&dataArgs);
  return output;
}

//TESTING
void assert(int line, bool b) {
    if (b) return;
    printf("The test on line %d fails.\n", line);
    exit(1);
}

char** split(char* input, const char* delimiter, int* outputlength) {
  //char* splitInput = (char*)malloc((strlen(input) + 1) * sizeof(char));
  char splitInput[(strlen(input) + 1)];
  splitInput[0] = ZERO;
  strcat(splitInput, input);
  char** output = (char**)malloc(sizeof(char*));
  char* substring = strtok(splitInput, delimiter);
  if (substring != NULL) *outputlength = 1;
  //*output = substring;
  *output = (char*)malloc((1+strlen(substring)) * sizeof(char));
  strcpy(*output, substring);
  while (substring != NULL) {
    substring = strtok(NULL, delimiter);
    if (substring != NULL) {
      output = realloc(output, ++(*outputlength) * sizeof(char*));
      *(output + *outputlength - 1) = (char*)malloc((1+strlen(substring)) * sizeof(char));
      strcpy(*(output + *outputlength - 1), substring);
      //*(output + *outputlength - 1) = substring;
    }
  }
  //free(splitInput);
  return output;
}

char* testGetOutputHelper(char* input, char** output) {
  int n;
  char** splitInput = split(input, " ", &n);
  char* args[n];
  for (int i = ZERO; i < n; i++) { //TO transform char** splitInput into char* args[]
    args[i] = *(splitInput + i);  //Because char* args[] is correct input format
  }
  free(*output);
  *output = getOutput(n, args); //calling getOutput with arguments in correct format
  for (int i = ZERO; i < n; i++) {
    free(*(splitInput+i));        //FREEING all malloced memory of splitInput
  }
  free(splitInput);
  //printf("getOutput %s: %s\n", input, *output);
  return *output;
}

void testGetOutput() {
  //Tests given in question sheet
  char* output = (char*)malloc(5 * sizeof(char));
  *output = 0;
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 7", &output), "0000 0111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -128", &output), "1000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 255", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 08", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -x0", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 255", &output), "1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int 10000000", &output), "0000 0000 1001 1000 1001 0110 1000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double -1.25 ", &output), "1011 1111 1111 0100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 1000 0000", &output), "-128") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int 0000 0000 1001 1000 1001 0110 1000 000", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 0000 0111", &output), "7") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double 1011 1111 1111 0100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000", &output), "-1.25") == ZERO);

  //My own tests
  //char
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 63", &output), "0011 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -42", &output), "1101 0110") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 127", &output), "0111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -128", &output), "1000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 128", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -129", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 4321", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -4321", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -X423", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char -4h", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char 74f", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise char ", &output), ERRORMESSAGE) == ZERO);
  //short
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short 32767", &output), "0111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short -32768", &output), "1000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short 32768", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short -32769", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short x43", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise short -53.41", &output), ERRORMESSAGE) == ZERO);

  //int
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int 2147483647", &output), "0111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int -2147483648", &output), "1000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int 2147483648", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int -2147483649", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise int 432.32", &output), ERRORMESSAGE) == ZERO);
  //long
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long 9223372036854775807", &output), "0111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long -9223372036854775808", &output), "1000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long 9223372036854775808", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long -9223372036854775809", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long -876.6", &output), ERRORMESSAGE) == ZERO);
  //long long

  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long long 9223372036854775807", &output), "0111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long long -9223372036854775808", &output), "1000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long long 9223372036854775808", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long long -9223372036854775809", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long long -876.6", &output), ERRORMESSAGE) == ZERO);
  //unisgned char
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 255", &output), "1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 0", &output), "0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 256", &output), "Input error.") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char -1", &output), "Input error.") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned char 74.1", &output), "Input error.") == ZERO);
  //unsigned short
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned short 65535", &output), "1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned short 0", &output), "0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned short 65536", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned short -1", &output), ERRORMESSAGE) == ZERO);
  //unsigned int
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned int 4294967295", &output), "1111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned int 0", &output), "0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned int 4294967296", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned int -1", &output), ERRORMESSAGE) == ZERO);
  //unsigned long
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long 18446744073709551615", &output), "1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long 0", &output), "0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long 18446744073709551616", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long -1", &output), ERRORMESSAGE) == ZERO);
  //unsigned long long
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long long 18446744073709551615", &output), "1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long long 0", &output), "0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long long 18446744073709551616", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise unsigned long long -1", &output), ERRORMESSAGE) == ZERO);
  //unsigned long long
  //float
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float 0", &output), "0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float 0.1", &output), "0011 1101 1100 1100 1100 1100 1100 1101") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float -0.1", &output), "1011 1101 1100 1100 1100 1100 1100 1101") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float 100000000000000000000000000000000000000", &output), "0111 1110 1001 0110 0111 0110 1001 1001") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float 1000000000000000000000000000000000000000", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float 0.7665789098397", &output), "0011 1111 0100 0100 0011 1110 1000 0100") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise float -0.f32", &output), ERRORMESSAGE) == ZERO);


  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double 0", &output), "0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double 0.1", &output), "0011 1111 1011 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1010") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double -0.1", &output), "1011 1111 1011 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1001 1010") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double 100000000000000000000000000000000000000000000000000000000000000000", &output), "0100 1101 0110 1110 0110 0010 1100 0100 1110 0011 1000 1111 1111 1000 0111 0010") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double 0.7665789098397", &output), "0011 1111 1110 1000 1000 0111 1101 0000 0111 1110 0111 0010 0001 0011 0011 0100") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise double -0.f32", &output), ERRORMESSAGE) == ZERO);

  //long double
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double 0", &output), "0000 0000 0000 0000 0000 0000 0000 0010 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double 0.1", &output), "0000 0000 0000 0000 0000 0000 0000 0010 0000 0000 0000 0000 0011 1111 1111 1011 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1101") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double -0.1", &output), "0000 0000 0000 0000 0000 0000 0000 0010 0000 0000 0000 0000 1011 1111 1111 1011 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1100 1101") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double 100000000000000000000000000000000000000000000000000000000000000000000000000000000000", &output), "0000 0000 0000 0000 0000 0000 0000 0010 0000 0000 0000 0000 0100 0001 0001 0010 1101 0010 1101 1000 0000 1101 1011 0000 0010 1010 1010 1011 1101 0110 0010 1100") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double 0.7665789098397", &output), "0000 0000 0000 0000 0000 0000 0000 0010 0000 0000 0000 0000 0011 1111 1111 1110 1100 0100 0011 1110 1000 0011 1111 0011 1001 0000 1001 1001 1001 1111 1000 0011") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise long double -0.f32", &output), ERRORMESSAGE) == ZERO);

  //combined types
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {}", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {long} 4321", &output), "0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0000 1110 0001") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {unsigned char} -5", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {int;long;float} -512 77771 3.14159265", &output), "1111 1111 1111 1111 1111 1110 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0010 1111 1100 1011 0100 0000 0100 1001 0000 1111 1101 1011") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {unsigned short;long long} -5 123151", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {unsigned long long;long double;char;float} 83473847 234.2342392 99 3.14159265358979", &output), "0000 0000 0000 0000 0000 0000 0000 0000 0000 0100 1111 1001 1011 0101 1011 0111 0000 0000 0000 0000 0000 0000 0000 0100 0000 0000 0000 0000 0100 0000 0000 0110 1110 1010 0011 1011 1111 0111 0001 1001 1010 0111 0111 0000 1111 0010 1011 0001 0110 0011 0100 0000 0100 1001 0000 1111 1101 1011") == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {char; int} 5 7", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {char; unsigned char} 5 7", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {float;int;long} ", &output), ERRORMESSAGE) == ZERO);
  assert(__LINE__, strcmp(testGetOutputHelper("./visualise {float;int 5.1 3 ", &output), ERRORMESSAGE) == ZERO);
  free(output);
}

char* testDecodeTypeHelper(char* input, char** output) {
  //splitting string input by spaces (as is done with args[] in main function)
  char2d datatypeArgs;
  datatype1d types;
  const char* delimiter = " ";
  datatypeArgs.arr = split(input, delimiter, &datatypeArgs.len);
  //adding len info to input struct
  datatypeArgs.sublen = (int*)malloc(datatypeArgs.len * sizeof(int));
  for (int i = ZERO; i < datatypeArgs.len; i++) {
    *(datatypeArgs.sublen + i) = (int)strlen(*(datatypeArgs.arr+i));
  }
  bool validInput = decodeType(&datatypeArgs, &types);
  destroyChar2d(&datatypeArgs);

  //converting types array to space separated numbers in a string
  char* temp = (char*)malloc((3+types.len * 2 * 2) * sizeof(char));
  //*output = realloc(temp, (3+types.len * 2 * 2) * sizeof(char));//*2 for whitespaces and *2 again because numbers might be 2 digit, but not 3 because only 13 datatypes
  //3+typ... is because when types.len is 0, we want to set temp to "-1" which has length 3
  if (validInput && types.len > 0) {
    int buffer = ZERO;
    for (int i = ZERO; i < types.len; i++) {
      if (*(types.arr + i) > NINE) {
        temp[i*2 + buffer] = ONECHAR;
        buffer++;
        *(types.arr + i) -= 10;
      }
      temp[i*2 + buffer] = (char)(*(types.arr + i) + ZEROCHAR);
      if (i < types.len-1) temp[i*2+1] = ' ';
    }
    temp[2*types.len - 1 + buffer] = ZERO;
  }
  else strcpy(temp, "-1");
  free(*output);
  *output = temp;
  free(types.arr);
  return *output;
}

void testDecodeType() {
  char* output = malloc(1);
  assert(__LINE__, strcmp(testDecodeTypeHelper("{long long;char;unsigned long long}", &output), "4 0 9") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("char", &output), "0") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("long double", &output), "12") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("float", &output), "10") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("{int}", &output), "2") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("{unsigned short;unsigned long long;int;unsigned int;unsigned char}", &output), "6 9 2 7 5") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("{}", &output), "-1") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("lon", &output), "-1") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("charchar", &output), "-1") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("int;int", &output), "-1") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("int,int", &output), "-1") == ZERO);
  assert(__LINE__, strcmp(testDecodeTypeHelper("{int; int}", &output), "-1") == ZERO); //illegal whitespace between semicolon and int
  free(output);
}

void testSignedChar() {

}

void testUnsignedChar() {

}

//END TESTING
void runTests() {
  testGetOutput();
  testDecodeType();
}

int main(int n, char* args[n]) {
  setbuf(stdout, NULL);
  if (n == 1) {
    runTests();
    printf("All tests pass.\n");
  }
  else {
    char* output = getOutput(n, args);
    printf("%s\n", output);
    free(output);
  }

  return 0;
}
