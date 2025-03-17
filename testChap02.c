int main(void)
{
	return ~(-5);
    //return 3 + 5*2;                 // fails @ '*' token
	//return 5*3+2;                   // produces 2,3,+,5,*, return
	//return 5*(3+2);                 // produces 2,3,+,5,*, return
    //return (3+2)*5;                 // produces 5,2,3,+,*, return
	//return -5 * (13 + -4) / 14 % 5; // fails @ '/' token
}