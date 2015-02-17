// This program is to process G-Code text file.
// Extract the contour path information and the infill path information.

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

using namespace std;

const double Del = 1e-6;

void 	textProcessing(const string &weldFile, const string &cutFile, const double layerThickness);
void 	commandTranslator(const string &command);
int 	extractInt(const string commandLine, char cha);
double 	extractDouble(const string commandLine, char cha);

int main()
{
	const string weldFile = "./extension_weld.gcode";		// Directory of gcode file for welding
	const string cutFile = "./extension_cut.gcode";		// Directory of gcode file for cutting
	const double layerThickness = 0.1;			// Thickness of each welding layer

	textProcessing(weldFile, cutFile, layerThickness);



	return 0;
}

void textProcessing(const string &weldFile, const string &cutFile, const double layerThickness)
{
	ifstream weldRead, cutRead;
	ofstream weldWrite, cutWrite;

	weldRead.open(weldFile);
	cutRead.open(cutFile);
	weldWrite.open("./weld.dat");
	cutWrite.open("./cut.dat");

	string commandLine;
	int layerCounter = 0;

	while ( ! weldRead.eof() ) {
		getline(weldRead, commandLine);

		//Step01: 只保留 G1 开头的命令代码
		if (commandLine[0] != 'G' || 1 != extractInt(commandLine, 'G')) 
			continue;
		//Step02: 丢掉不含 X Y Z 的命令代码
		if ( abs(extractDouble(commandLine, 'X')) < Del &&
				abs(extractDouble(commandLine, 'Y')) < Del &&
				abs(extractDouble(commandLine, 'Z')) < Del )
			continue;
		//Step03: 找到第一层的第一行命令
		if ( layerCounter == 0 && 
			abs(extractDouble(commandLine, 'Z') - layerThickness) > Del)
			continue;
		else if ( layerCounter == 0 ) {
			layerCounter ++;		// 第一层第一个命令已找到
			weldWrite << 'H' << layerCounter << endl;
			weldWrite << commandLine << endl;
		}
		//Step04: 依次找到剩下每层的起点命令，并标注出来
		if ( abs(extractDouble(commandLine, 'Z')) > Del &&
			abs(extractDouble(commandLine, 'Z') - (layerCounter + 1)*layerThickness) > Del )
			continue;
		else if ( abs(extractDouble(commandLine, 'Z') - (layerCounter + 1)*layerThickness) < Del ){
			layerCounter ++;
			weldWrite << 'H' << layerCounter << endl;
		}


		weldWrite << commandLine << endl;
	}
	cout << "Final welding layer counter: " << layerCounter << endl;
	if (layerCounter == 0) {
		cout << "Setting of layer thickness is Wrong!!!" << endl;
		return;
	}

	layerCounter = 0;
	while ( ! cutRead.eof() ) {
		getline(cutRead, commandLine);

		//Step01: 只保留 G1 开头的命令代码
		if ( commandLine[0] != 'G' || 1 != extractInt(commandLine, 'G') )
			continue;
		//Step02: 丢掉不含 X Y Z 的命令代码
		if ( abs(extractDouble(commandLine, 'X')) < Del &&
			abs(extractDouble(commandLine, 'Y')) < Del &&
			abs(extractDouble(commandLine, 'Z')) < Del )
			continue;
		//Step03: 找到第一层的第一个命令
		if ( layerCounter == 0 && abs(extractDouble(commandLine, 'Z') - layerThickness) > Del)
			continue;
		else if (layerCounter == 0) {
			layerCounter ++;
			cutWrite << 'H' << layerCounter << endl;
			cutWrite << commandLine << endl;
		}
		//Step04: 依次找到剩下每层的起点命令，并标注出来
		if ( abs(extractDouble(commandLine, 'Z')) > Del &&
			abs(extractDouble(commandLine, 'Z') - (layerCounter + 1)*layerThickness) > Del )
			continue;
		else if ( abs(extractDouble(commandLine, 'Z') - (layerCounter + 1) * layerThickness) < Del ) {
			layerCounter ++;
			cutWrite << 'H' << layerCounter << endl;
		}

		cutWrite << commandLine << endl;
	}
	cout << "Final cutting layer counter: " << layerCounter << endl;
	if (layerCounter == 0) {
		cout << "Setting of layer thickness is Wrong!!!" << endl;
		return;
	}

	weldRead.close();
	cutRead.close();
	weldWrite.close();
	cutWrite.close();
}

// Get a integer number following the specified character from a command string
int extractInt(const string commandLine, char cha)
{
	int ind;
	ind = commandLine.find(cha);
	if ( ind < 0 ) return 0;
	return strtol(&commandLine[ind+1], NULL, 10);
}

// Get a 'double' number following the specified character from a command string 
double extractDouble(const string commandLine, char cha)
{
	int ind;
	ind = commandLine.find(cha);
	if ( ind < 0 ) return 0;
	return strtod(&commandLine[ind+1], NULL);
}

void commandTranslator(const string &command)
{
	if (command[0] != 'G')
		return;
	cout << command << endl;
}