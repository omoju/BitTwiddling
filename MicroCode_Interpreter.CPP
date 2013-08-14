/*=========================================================================
  Program:   Micro_code interpreter (Prog HW 4)  ==   Class: EECE 6278
  Author:    Omoju Miller                        ==   Language:  C		
 =========================================================================*/
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

// Global vars
int mem[512]={0}, ic, ir, ac, addr , op_code, GLOBAL_TRACE=0, R0, RJ, code, end_sequence = 0;
int micro_code[8][4]={{1,8,5,0},{9,1,8,0},{1,5,6,0},{7,8,9,6}
                       ,{9,8,2,0},{1,4,7,8},{1,3,4,2},{8,9,6,10}};

//functions
int preload(void);
void load(void);
void run(void);
int dump(void);
void trace(void);
void JCL_Info(void);
void JCL_Intro(void);
void exec_micro_code(void);

void main()
{	
	char cmd[100] = ""; preload(); ic = 0, addr = 0, ac = 00; // Initialize everything to zero
	JCL_Intro(); JCL_Info();
	while(ic != 512) // runs the JCL
	{   cout << ">"; cin >> cmd; cout << ">";
		if(cmd[0] != '*'){ cerr << "Invalid cmd" << endl; JCL_Info(); }
		else if(cmd[0]=='*' && cmd[1]=='l' && cmd[2]=='o' && cmd[3]=='a' && cmd[4]=='d')
			{ load(); }
		else if(cmd[0]=='*' && cmd[1]=='g' && cmd[2]=='o'){ run(); }
		else if(cmd[0]=='*' && cmd[1]=='d' && cmd[2]=='u' && cmd[3]=='m' && cmd[4]=='p')
			{ dump(); }
		else if(cmd[0]=='*' && cmd[1]=='t' && cmd[2]=='r' && cmd[3]=='a' && 
		   cmd[4]=='c' && cmd[5] == 'e'){ GLOBAL_TRACE = 1; }
		else if(cmd[0]=='*' && cmd[1]=='n' && cmd[2]=='o' && cmd[3]=='t' && 
		   cmd[4]=='r' && cmd[5] =='a' && cmd[6]=='c' && cmd[7]=='e'){ GLOBAL_TRACE = 0; }
		else if(cmd[0]=='*' && cmd[1]=='e' && cmd[2]=='n' && cmd[3]=='d'){ exit(0); }
		else { cerr << "Invalid cmd" << endl; JCL_Intro(); }
	}
}

//Describes the kind of format the Job control lines must be 
void JCL_Info(void)
{ cout << "Please enter a cmd in this form> *cmd" << endl; }

void JCL_Intro(void)
{ 
  cout <<endl<<setw(20)<<"********** AVAILABLE COMANDS *********"<<endl;
  cout <<setw(20)<<"load"<<endl<<setw(20)<<"go"<<endl<<setw(20)<<"dump"<<endl;
  cout <<setw(20)<<"trace"<<endl<<setw(20)<<"notrace"<<endl<<setw(20)<<"end"<<endl<<endl; 
}

//loads the input file into the virtual memory
int preload(void)
{	
	int Line_Number, End=0;

	ifstream fin("memory_file_input.txt");
	if(!fin){ cout << "Cannot open file, ending program."<<endl; exit(1);}

	for(int x = 0; x <= 511; x = x + 8)
	{   fin >> dec >> Line_Number;
		for(int y = 0; y <= 7; y++){fin >> oct >> mem[(Line_Number-100)*8+y];}		
	}
	fin >> oct >> End;
	fin.close();
	return End;
}

// set the opcodes
void load(void)
{	
	ir = mem[ic];
	if(ir < 01000){op_code = 00000;addr = ir - 00000;}
	if(ir >= 01000 && ir < 02000){op_code = 01000;addr = ir - 01000;}
	if(ir >= 02000 && ir < 03000){op_code = 02000;addr = ir - 02000;}
	if(ir >= 03000 && ir < 04000){op_code = 03000;addr = ir - 03000;}
	if(ir >= 04000 && ir < 05000){op_code = 04000;addr = ir - 04000;}
	if(ir >= 05000 && ir < 06000){op_code = 05000;addr = ir - 05000;}
	if(ir >= 06000 && ir < 07000){op_code = 06000;addr = ir - 06000;}
	if(ir >= 07000){op_code = 07000;addr = ir - 07000;}

	if(GLOBAL_TRACE==1){trace();}
	code = 1; exec_micro_code(); //ic++;
}

// This determines the micro-code and initiates its execution
void run(void)
{
	for(int i=0; i<4; i++)
	{
		if(op_code < 01000) { code = micro_code[0][i]; exec_micro_code(); }
		else if (op_code >= 01000 && op_code <= 02000) 
			{ code = micro_code[1][i]; exec_micro_code(); }
		else if (op_code >= 02000 && op_code <= 03000) 
			{ code = micro_code[2][i]; exec_micro_code(); }
		else if (op_code >= 03000 && op_code <= 04000) 
			{ code = micro_code[3][i]; exec_micro_code(); }
		else if (op_code >= 04000 && op_code <= 05000) 
			{ code = micro_code[4][i]; exec_micro_code(); }
		else if (op_code >= 05000 && op_code <= 06000) 
			{ code = micro_code[5][i]; exec_micro_code(); }
		else if (op_code >= 06000 && op_code <= 07000) 
			{ code = micro_code[6][i]; exec_micro_code(); }
		else if (op_code >= 07000) 
			{ code = micro_code[7][i]; exec_micro_code(); }
	}
	if(GLOBAL_TRACE==1){trace();} 
	code = 1; exec_micro_code(); // ic++
}

// Execution of the micro-code operations
void exec_micro_code(void)
{ 
	switch(code)
	{
	case 0:{ load(); break;}
		case 1:{ ic = addr; break;}
		case 2:{ exit(0); break;}
		case 3:{ mem[addr] = 010000-ac; break;}
		case 4:{ mem[addr] = ac; break;}
		case 5:{ mem[addr] = ic; break;}
		case 6:{ cout << oct << mem[addr] << endl; break;}
		case 7:{ ac = ac + mem[addr]; break;}
		case 8:{ ic = ic + 1; break;}
		case 9:{ mem[addr] = 01000 + ic; break;}
		case 10:{ if(ac <= 03777){end_sequence = 1;} break;}
	}
}

// Write the virtual memory out to a file
int dump(void)
{	
	int index=100;
	ofstream fout("memory_file_output.txt");if(fout.fail()){return 0;}
	for(int x = 0; x <= 511; x = x + 8)
	{
		if(mem[x]+mem[x+1]+mem[x+2]+mem[x+3]+mem[x+4]+mem[x+5]+mem[x+6]+mem[x+7] != 0)
		{
			fout << dec << index << "   ";
			for(int i=0; i<8;i++)
			{   if(i==3)fout << " ";
				fout << setw(4) << setfill('0') << oct << mem[x+i] <<" "; 
			}fout <<endl;
		}index++;
	}
	fout << oct << 0777;
	fout << flush;
	fout.close();
	return 0;
}

//Trace variable values
void trace()
{
	cout << "ic: "   << setw(4) << setfill ('0') << dec << ic   << " ";
	cout << "ir: "   << setw(4) << setfill ('0') << oct << ir   << " ";
	cout << "ac: "   << setw(4) << setfill ('0') << oct << ac   << " ";
	cout << "addr: " << setw(4) << setfill ('0') << oct << addr << " ";
	cout <<"mem[" << oct << addr << "]: " << setw(4) << setfill ('0') << oct << mem[addr] << endl;
}

/* OUTPUT

  
********** AVAILABLE COMANDS *********
                load
                  go
                dump
               trace
             notrace
                 end

Please enter a cmd in this form> *cmd
>*trace
>>*load
>ic: 0000 ir: 3123 ac: 0000 addr: 0123 mem[123]: 2465
>*go
>123
ic: 0083 ir: 0123 ac: 0000 addr: 0123 mem[123]: 0123
ic: 0083 ir: 0123 ac: 0000 addr: 0123 mem[123]: 0123
>*load
>ic: 0083 ir: 0123 ac: 0000 addr: 0123 mem[123]: 0123
>*go
>ic: 0084 ir: 0000 ac: 0000 addr: 0000 mem[0]: 3123
ic: 0000 ir: 0000 ac: 0000 addr: 0000 mem[0]: 3123
>*end
>Press any key to continue

*/