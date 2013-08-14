#pragma region Definitions & Includes
//
// Definitions & Includes
//

#include <iostream>									// Andy Lemmon, Omoju Miller, Joseph Qualls
#include <iomanip>									// EECE 6278 Final Project
#include <fstream>									// 4/25/06
#include <sstream>
#include <vector>
#include <time.h>
using namespace std;

#define ERROR				-1
#define OPCODE(x) 			((x >> 12) & 0x07)		// Decode OpCode from int
#define OPER(x) 			(x & 0xFFF)				// Decode Operand from int
#define INSTRUCTION(x, y)	((x << 12) + y)			// Build an int x=OC y=OP
#define MAIN_MEM_SIZE		512
#define VIRT_MEM_SIZE		4096
#define LINE_LENGTH			56
#define LINES_PER_BLOCK		64
#define BLOCKSIZE			(LINE_LENGTH * LINES_PER_BLOCK + 2)	// + 2 for blank line
#define PAGE_TABLE_SIZE		8
#define INSTR_PAGE(x)		((x >> 9) & 0x07)
#define OPER_TRANSLATE(x)	(x & 0x1FF)

enum INSTRUCTIONS {	HTR, TRA, TMI, RJP,	RDW, WRW, ADD, STN	};
													
typedef struct _CpuContext							
{	
	int PC;	
	int IR; 
	int AC;	
	int CP;

}	CpuContext;
#pragma endregion

#pragma region Global Variables
//
// Global Variables
//

int mem[MAIN_MEM_SIZE];								
int PT[PAGE_TABLE_SIZE];
bool debugtrace;									
CpuContext ctx;										
char *filename;
int numswaps;
int numinstr;
int swapmin;
int swapmax;
float swapavg;
long totalswap;

ostringstream maindumpstr;							
ostringstream tempdumpstr;							
ostringstream rundumpstr;							
#pragma endregion

#pragma region Function Prototypes
//
// Function Prototypes
//

int load();											
int run(const int instruction);						
void dump(int index, ostream &out, ostringstream &dumpstr);	

int parseinputfile(char *filename, const int pagenum = 0);
void writeoutputfile();

void pagefault(const int address);
int pt_exist(const int nextpage);
int pt_free(void);
int writepage(const int pagenum);
void loadpage(const int pageneeded);

void dumpall(ostream &out, ostringstream &dumpstr);
void writecurrentpagetoswap();
void printcontext(bool last = false);
void printpagetable();

#pragma endregion

#pragma region Main Method
//
// Main Method
//

int main(int argc, char *argv[])
{
	int ret;
	clock_t starttime, endtime;
	FILE *swap = fopen("swapfile.txt", "w+");
	fclose(swap);
	//
	numswaps = 0;
	numinstr = 0;
	swapmin = INT_MAX;
	swapmax = 0;
	swapavg = 0.0;
	totalswap = 0;
	memset(&mem, 0, sizeof(mem));					// Format the memory
	memset(&ctx, 0, sizeof(ctx));					// Format the registers
	memset(&PT, -1, sizeof(PT));					// Format the Page Table

	if( argc != 2 && argc != 3 )					// Check argument validity
	{												//
		cout << "usage: memmgr infile [all]" << endl;
		return ERROR;								
	}												//

	filename = argv[1];
	if(argc == 3)	
	{
		debugtrace = true;							// Set Debugtrace flag
	}

	ret = parseinputfile(argv[1]);					// Parse the input file
	if( ret < 0 )	
	{
		return ERROR;						
	}

	cout << endl << ">> Memory Before Run: " << endl;
	dumpall(cout, maindumpstr);						// Print out memory before run
	
	ctx.PC = 0;
	cout << endl << ">> Running Program at [" << oct << ctx.PC << "]...." << endl;

	starttime = clock();

	// Fetch, decode, execute:
	//

	while(ctx.PC < VIRT_MEM_SIZE)							
	{												
		load();										
		if (debugtrace)								
			printcontext();							
		run(ctx.IR);
		if (debugtrace)								
			printcontext(true);						
		dump(ctx.PC, cout, rundumpstr);				
		ctx.PC++;									
		if(ctx.PC <= VIRT_MEM_SIZE) pagefault(ctx.PC);

		numinstr++;
	}												

	endtime = clock();

	cout << endl << ">> Memory After Run: " << endl;// Print out memory after run
	dumpall(cout, maindumpstr);						// Dump to console

	writeoutputfile();

	cout << dec;
	cout << endl << ">> Summary: " << endl;
	cout << "Inst Count ==> " << (numinstr - 1) << endl;
	cout << "PF Count   ==> " << numswaps << endl;
	cout << "Run Time   ==> " << (endtime - starttime) << " ms" << endl;
	cout << "PF Timing  ==> " << ((swapmin == INT_MAX) ? 0 : swapmin) << " ms / " << swapmax << " ms / " << swapavg << " ms" << endl;
	return 0;
}
#pragma endregion

#pragma region BasicIO Methods
//
// Basic Input Output Methods
//

// Load an instruction into memory
int load()
{		
	int addr = OPER_TRANSLATE(ctx.PC);
	ctx.IR = mem[addr];								// Load the instruction register
	return addr;									// Return PC
}

// Run the instruction
int run(const int instruction)
{
	char *opdesc = NULL;							// 
	int opcode = OPCODE(instruction);				// Decode the opcode
	int operand = OPER(instruction);				// Decode the operand

	pagefault(operand);								// tarzan kicked ape
	operand = OPER_TRANSLATE(operand);

	switch(opcode)									// Perform the required processing
	{												//
		case HTR:	opdesc = "HTR, ";				// Halt (set PC to invalid value)
			ctx.PC = 4096;					break;	//
		case TRA:	opdesc = "TRA, ";				// Transfer to (X)
			ctx.PC = operand;				break;	//
		case TMI:	opdesc = "TMI, ";				// Transfer to (X) if AC neg
			if(ctx.AC >= 0x7FF) ctx.PC = operand;	break;
		case RJP:	opdesc = "RJP, ";				// Jump to subroutine
			mem[operand+1] = INSTRUCTION(TRA, ctx.PC);
			ctx.PC = (operand - 1);			break;	//
		case RDW:	opdesc = "RDW, ";				// Read AC value into word at (X)
			mem[operand] = ctx.AC;			break;	//
		case WRW:	opdesc = "WRW, ";				// Write word at (X) into AC
			ctx.AC = mem[operand];			break;	//
		case ADD:	opdesc = "ADD, ";				// Add (X) to AC value
			ctx.AC = ctx.AC + operand;		break;	//
		case STN:	opdesc = "STN, ";				// Store negative of AC
			mem[operand] = ~(ctx.AC) + 1;	break;	//
	}												//
	if(debugtrace)									// Print debugging info
		cout << ">> Executing " << opdesc << "Operand = " << operand << endl;
	return 0;
}

// Dump output to either console or file
void dump(int index, ostream &out, ostringstream &dumpstr)
{
	if( (index % 8) == 0 )							// Start of line
	{
		dumpstr << setw(1) << oct << ctx.CP;
		dumpstr << setfill('0') << setw(3) << oct << index << " ";
	}
	int addr = OPER_TRANSLATE(index);
	dumpstr << setfill('0') << setw(1) << oct << (int) OPCODE(mem[addr]);
	dumpstr << setfill('0') << setw(4) << oct << OPER(mem[addr]) << " ";	
	if( (index % 8) == 3 )	dumpstr << " ";			// Space in middle of line
	if( (index % 8) == 7 )							// End of line: write to file
	{
		out << dumpstr.str() << endl;
		dumpstr.str("");							//
	}												// Only print nonzero lines
}
#pragma endregion

#pragma region FileIO Methods
//
// File Input Output Methods
//

// Parse file content and make a list of instructions
int parseinputfile(char *filename, const int pagenum)
{
	istringstream parser; string currline, token;	// Declare variables
	vector<string> instlist; 
	int addr, index, linesread;
	ifstream infile (filename);						// Create input file

	int seekpos = pagenum * BLOCKSIZE;

	infile.seekg(seekpos);
	if( infile.fail() )
	{
		cout << "Seek failed." << endl;
		return -1;
	}

	linesread = 0;
	memset(&mem, 0, sizeof(mem));					// Format the memory
	if( !infile.good() ){ 							// Ensure file was opened
		cout << "Could not open " << filename;	
		return -1;		}							// ERROR: Couldn't open file
	while( infile.good() && (linesread < LINES_PER_BLOCK) )
	{							
		getline(infile, currline);					// Put one line into currline
		linesread++;
		instlist.clear();							// Clear instruction list
		parser.clear();								// Clear parser object
        parser.str(currline);						// Attach parser to currline
		addr = index = 0;							// Clear index & addr
		while( parser >> token ){ 					// Loop through currline
			if(index == 0){							// 1st token is address
				addr = strtol(token.c_str(), NULL, 8);
			} else {								// Other tokens: instructions
				instlist.push_back(token);			// Add instruction to list
			}
			index++;								// Increment token counter
		}											// Build mem-line from list
		addr = OPER_TRANSLATE(addr);
		if( addr >= 0 && addr <= 504 && instlist.size() ){
			for( int i=0; i < (int) instlist.size(); i++ ){
				int opcode  = strtol(instlist[i].substr(0,1).c_str(), NULL, 8);
				int operand = strtol(instlist[i].substr(1,5).c_str(), NULL, 8);
				mem[i+addr] = INSTRUCTION(opcode, operand);
			}										// This loop makes an int
		}											// out of each opcode & oper
	}												// and loads into mem[]
	return 0;
}

// Parse the swap file and write all pages in the proper order
void writeoutputfile()
{
	cout << ">> Writing Output File" << endl;

	writecurrentpagetoswap();
	ofstream outfile("mem.txt");							// Open output file

	printpagetable();
	cout << "OUT[";
	for(int i = 0; i < PAGE_TABLE_SIZE; i++)
	{	
		loadpage(i);										//
		dumpall(outfile, maindumpstr);						// Dump to persistent store
		cout << i;
		if( i < (PAGE_TABLE_SIZE-1) )	
			cout << ", ";
	}
	cout << "]" << endl;
}
#pragma endregion

#pragma region Swap Methods
//
// Swap Methods
//

// Determine whether a page fault needs to occur, and perform the appropriate action
void pagefault(const int address)
{
	int pageneeded, swaptime;
	clock_t starttime, endtime;

	// Is the necessary page already loaded into memory?
	//

	pageneeded = INSTR_PAGE(address);

	if( debugtrace )
		cout << "Page needed = " << pageneeded << endl;

	if (pageneeded == ctx.CP)
	{
		// The necessary page is already loaded.  No action needed.
		//

		return;
	}
	else
	{
		numswaps++;
		cout << dec;
		cout << ">> Page Fault <" << pageneeded << ">....";
		starttime = clock();

		// Swap out the current memory.
		//

		writecurrentpagetoswap();

		// Now, load the new page into memory.
		//

		loadpage(pageneeded);

		// Gather Statistics
		//

		endtime = clock();
		swaptime = (endtime - starttime);
		if( swaptime < swapmin )
			swapmin = swaptime;
		if( swaptime > swapmax )
			swapmax = swaptime;
		totalswap += swaptime;
		swapavg = (float) totalswap / (float) numswaps;
		cout << "(" << swaptime << ")" << endl;
		cout << oct;
	}

	return;
}

// Check to see if current mem[] is in swap file
int pt_exist(const int nextpage)
{
	for(int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		if(PT[i] == nextpage)					// tarzan got banana
		{
			return i;
		}
	}

	return -1;
}

// Look for a free slot in the swap file
int pt_free(void)
{
	for(int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		if(PT[i] == -1)							// tarzan eat banana
		{
			if( debugtrace )
				cout << "Found Free page at " << i << endl;
			return i;
		}
	}

	return -1;
}

// Write a page to the swap space
int writepage(const int pagenum)
{
	ofstream swapfile("swapfile.txt", ios::in | ios::out );
	if( swapfile.fail() )
	{
		cout << "Opening swapfile failed." << endl;
		return -1;
	}

	int seekpos = pagenum * BLOCKSIZE;

	swapfile.seekp(seekpos, ios::beg);
	if( swapfile.fail() )
	{
		cout << "Seek Failed." << endl;
		return -2;
	}

	dumpall(swapfile, tempdumpstr);
	return 0;
}

// Load a page from either swap or input file
void loadpage(const int pageneeded)
{
	int pageloc = pt_exist(pageneeded);
	if(pageloc != -1)
	{
		// This page is in the swap file, so load it from there
		//

		parseinputfile("swapfile.txt", pageloc);
	}
	else
	{
		// This page is not in the swap file, must load from the input
		//

		parseinputfile(filename, pageneeded);
	}

	ctx.CP = pageneeded;
}
#pragma endregion

#pragma region Utility Methods
//
// Utility Methods
//

// Dump entire memory array to console or file
void dumpall(ostream &out, ostringstream &dumpstr)
{
	for(int i = 0; i < 512; i++){	dump(i, out, dumpstr);	}
	out << endl;
}

// Write the current memory contents to the swap file
void writecurrentpagetoswap()
{
	int temp = pt_exist(ctx.CP);
	if (temp != -1)
	{
		// Current Page is already in the swap file.  Overwrite.
		//

		writepage(temp);
	}
	else
	{
		// Current Page is not already in swap file.  Write current page into a free slot.
		//

		temp = pt_free();
		writepage(temp);
		PT[temp] = ctx.CP;
	}
}

// Print out the value of all CPU context registers
void printcontext(bool last)
{	
	if( ctx.PC < VIRT_MEM_SIZE )					// Index validity check
	{												//
		int addr = OPER_TRANSLATE(ctx.PC);
		cout << "PC="  << setfill('0') << setw(4) << oct << ctx.PC << " ";
		cout << "IR="  << ctx.IR << " ";			// IR
		cout << "AC="  << ctx.AC << " ";			// AC
		cout << "MEM=" << mem[addr] << endl;		// MEM
		printpagetable();
		if( last )	cout << endl;					// Add a carriage return
	}
}

// Print out the entire page table
void printpagetable()
{
	cout << "PT=[";
	for(int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		if( i < (PAGE_TABLE_SIZE-1) )
		{
			if( PT[i] == -1 )
				cout << "*, ";
			else
				cout << PT[i] << ", ";
		}
		else
		{
			if( PT[i] == -1 )
				cout << "*]" << endl;
			else
				cout << PT[i] << "]" << endl;
		}
	}
}
#pragma endregion
