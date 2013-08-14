/*=========================================================================
  Program:   Memory load and dump (Prog HW 3)  ==   Class: EECE 6278
  Author:    Omoju Miller					   ==   Language:  C		
 =========================================================================*/
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#define MAX 20
#define MEM_MAX 1024
#define addr_size 8

struct{ 
	int ic;
	int opcode;
} DATAIN[MAX], DATAOUT, R0;
int ic=0, end=0, index=0, GLOBAL_TRACE=0, IR = 0, AC = 0, opcode, addr, trace_index;
unsigned int IOSTATUS[4]={0}; int mem[MEM_MAX];
void preload(ifstream fin);
void load();
void run();
void dump(void);
void dump_2(ofstream fout);
void trace(void);

void main()
{	ifstream fin; ofstream fout; int key;
	fin.open("memory_file_input.txt");	fout.open("memory_file_output.txt");
	if (!fin) { cout << "Cannot open file, ending program." <<endl;  exit(1); }
	preload(fin);	fin.close();

	cout <<"For trace enter 1 or 0 for no trace ";
	cin >> GLOBAL_TRACE; 
	trace_index = 0;

	if(GLOBAL_TRACE)
	{	for(int i=0; i<end*addr_size; i++)
		{	load(); 
			run();  trace();
			dump(); cout<< endl; trace_index++; 
		}
	}
	else
	{   dump_2(fout); cout << "Memory dumped to file 'memory_file_output.txt' "<<endl;
		for(int i=0; i<end*addr_size; i++)
		{	load(); run(); dump();	}	
	}

	cout << "To exit press any key ";
	cin >> key; exit(0);
}
// Run acts like an interpreter decomposing memeory into opcode and address
void run()
{   IR = mem[ic];
	if(IR/1000 == 1){opcode = 1; addr = IR%1000;} if(IR/2000 == 1){opcode = 2; addr = IR%2000;}
	if(IR/3000 == 1){opcode = 3; addr = IR%3000;} if(IR/4000 == 1){opcode = 4; addr = IR%4000;}
	if(IR/5000 == 1){opcode = 5; addr = IR%5000;} if(IR/6000 == 1){opcode = 6; addr = IR%6000;}
	if(IR/7000 == 1){opcode = 7; addr = IR%7000;} if(IR/8000 == 1){opcode = 8; addr = IR%8000;}
	if(IR/9000 == 1){opcode = 9; addr = IR%9000; }

	switch(opcode)
	{	case 0:{ mem[ic] = addr; break;}
		case 1:{ mem[ic] = mem[addr]; break;}
		case 2:{ if(AC < 0) mem[ic] = mem[addr]; break;}
		case 3:{mem[ic] = mem[addr] + 1; break;}
		case 4:{ cout << "Enter a value "; cin >> mem[addr]; break;}
		case 5:{ cout << mem[addr]; break;}
		case 6:{ AC = mem[addr] + AC; break;}
		case 7:{ AC = -AC; break;}
	} 
}

// a function that preloads all the memory at once allow for the program to commence
void preload(ifstream fin)
{	int start = IR, i = 0, j, k = 9999999, l;
	fin >> dec >> DATAIN[IR].ic;
	index = 0;
	do 
	{	for(j=0; j<addr_size; j++)
		{	fin >>  dec >> l;   mem[index++]= l;	}

		if(IR == 0){	k = DATAIN[IR].ic; }

		IR++;  fin >> dec >> DATAIN[IR].ic;

	}while(k!= DATAIN[IR].ic && !fin.eof());
	end = IR;	IR =0;
}

//New Load - initializes mem[ic], checks bit 3 in IOSTATUS and 
//fetches the instr from DATAIN into a temp reg R0 and into mem[ic] if bit 3 is high
//Bit 3 goes high as soon as prog is ready to fetch data from DATAIN reg.
void load()
{	IOSTATUS[2] = 01111;
	R0 = DATAIN[index];	
	if (IOSTATUS[2] & 01111) //check bit 3 in IOSTATUS to see if high
	{	;} //mem[ic] = R0;	}
}

//modify dump s.t. the interpreter points the ic to current mem[ic] and waits for next octal byte from
//R0 to DATAOUT register, then incs mem[ic] and waits for next octal byte input
void dump(void)
{	IOSTATUS[3] = 01111;
	DATAOUT = R0;
	ic++; index++;
	IOSTATUS[3] = 0, IOSTATUS[2] = 0; //read-write change the bits to low
}

// dump_2 writes the dump to a file
void dump_2(ofstream fout)
{	int index = 0;
	for(int i=0; i<end; i++)
	{   fout << setw(3) << setfill('0') << i;
		for(int j=0; j<addr_size; j++)
		{ if(index % 4 == 0) {fout << " ";}
		  fout << setw(4) << mem[index++] << " ";
		}fout << endl;
	}
}

//Trace variable values
void trace()
{	if(!trace_index)
		cout<<setw(10)<<"IC"<<setw(10)<<"IR"<<setw(20)<<"AC"<<setw(20)<<"MEM[addr]"<<setw(10)<<"addr"<<endl; 
	cout<<setw(10)<<ic<<setw(10)<<dec<<IR;
	cout<<setw(20)<<dec<<AC<<setw(20)<<dec<<mem[addr]<<setw(10)<<dec<<addr<<endl; 
	

}
