#include <fstream>
#include <iostream>
#include <windows.h>
#include <vector>
#include <time.h>
#include <sstream>
#include <regex>

using namespace std;

const int FINISH_FLAG = 19900904;
const int cycle_number = 300;

struct result_info{
	int nc;
	double time;
	double stress;
	double strain;
	double Astress;
} ri ;

string getTime();

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

///main program
int main(int argc, char const *argv[])
{
	system("md result");
	bool isRead = false;
	string result_file = "DATA.bin";
	fstream _file;
	_file.open(result_file, ios::in | ios::binary);

	if(!_file){
		isRead = false;
	}
	else{
		int FINISH_FLAG_TEMP = 0;
		_file.read((char*) &FINISH_FLAG_TEMP ,4);
		if(FINISH_FLAG_TEMP != FINISH_FLAG){
			isRead = false;
		}
		else{
			isRead = true;
		}
	}

	_file.close();

	if (!isRead){
		cout << "----------------Reducing GDS file, Plase wait!---------------" << endl; 
		if(argc == 1){
			cout << " ---------------- ERROR ----------------" << endl;
			cout << "1. run by this way : gds.exe yourgdsfile.gds"<< endl;
			cout << "Exsample: gds.exe 1.gds."<< endl;
			cout << "2. drag GDS file on the this exe icon."<< endl;
			system("PAUSE");
		}
		else{

			int mm = 0;
			string BIN_path = argv[1];
			ifstream fin(BIN_path.c_str(),ios::binary);
			ofstream fout(result_file.c_str(),ios::binary);
			string line;
			//escape header line
			while(getline(fin, line)){
				if((int)line.find("Stage Number") > 0)		
					break;
			}
			// total number of data line
			int total_dataline = 0;
			// t1 time at program beginning
			long t1 = GetTickCount(); // #include <windows.h>
			// read main data
			regex rx("\""); // delete "

			vector<std::string> v;

			while(getline(fin, line)){
				// delete "
				//cout << line << endl;
				line = regex_replace(line, rx, "");
				// splite with ,
				v.clear();
				SplitString(line, v, ",");

				//build data structure
				ri.nc = stoi(v[0]);
				ri.time = stod(v[1]);
				ri.stress = stod(v[30]);
				ri.strain = stod(v[36]);
				ri.Astress = stod(v[33]);
				//cout << ri.nc << ri.time << ri.stress << ri.strain << endl;
				// escapte stage 1
				if (1 != ri.nc){
					// write in file BIN.data with binary
					fout.write((char*)&ri, sizeof(ri));
					total_dataline += 1;
				}
			}
			// at data file begin record number of data line to verify
			/////////
			fout.clear(); //clean EOF status
			fout.seekp(0,std::ios::beg); // move to start of file	
			fout.write((char*)&FINISH_FLAG, sizeof(FINISH_FLAG));//reord FINISH FLAG
			fout.write((char*)&total_dataline, sizeof(total_dataline));//record number of data line 
			/////////
			// CLOSE FILE 
			fout.close();
			//t2 time at program finosh
			long t2 = GetTickCount();
			cout << "time: " << (t2-t1)/1000.0 << " sec" << endl; // print
			system("gds.exe");
		}
	}
	else{
		//read data.bin here
		cout << "----------------Reading Already GDS file, Plase input!---------------" << endl; 
		int total_data = 0;
		int FINISH_FLAG_TEMP = 0;
		// PATH
		string input_temp;
		string md_path = "result\\";


		ifstream fin(result_file.c_str(),ios::binary);

		fin.read((char*) &FINISH_FLAG_TEMP ,sizeof(FINISH_FLAG_TEMP));
		if(FINISH_FLAG_TEMP != FINISH_FLAG){
			cout << "BIG ERROR HERE"<< endl; 
			system("PAUSE");
			return 1;
		}
		fin.read((char*) &total_data ,sizeof(total_data)); 
		cout << "total data line : " << total_data - cycle_number + 5 << endl;
		std::string print_string = "";
		std::stringstream ss;

	    std::string csv_string = "";
		std::stringstream cvs;

	    int index_data;
		while(true){
			print_string = "";
			csv_string = "";
			cout << "input cycle: please < " << total_data << endl;
			getline(std::cin, input_temp);
			index_data = atoi(input_temp.c_str());
			if( index_data > total_data)
				continue;

			string out_filename = md_path + input_temp + getTime();

			ofstream fout(out_filename.c_str(),ios::out);
			//to csv file header
			fout << "stage,time,stress,strain,Ax stress\n" << std::flush;

			int pianyi =  index_data * sizeof(ri);
			fin.seekg(pianyi, ios::beg);

			for (int q = 0; q < cycle_number; q++)
			{
				ss.str("");
				cvs.str("");
				fin.read( (char *)&ri, sizeof(ri) );
				ss.precision(10);
				ss << "stage:" << ri.nc 
				   << ",time:" << ri.time
				   << ",stress:" << ri.stress
				   << ",strain:" << ri.strain
				   << ",Axstress:" << ri.Astress
				   << '\n' << std::flush;
				cvs.precision(10);
				cvs << ri.nc 
				   << "," << ri.time
				   << "," << ri.stress
				   << "," << ri.strain
				   << "," << ri.Astress
				   << '\n' << std::flush;				

				print_string += ss.str();
				csv_string += cvs.str();
			}
			cout << print_string << endl;
			fout << csv_string;
			fout.close();
		}
		fin.close();

	}
	return 0 ;
}

string getTime()
{
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "-aT-%Y%m%d-%H%M%S.csv",localtime(&timep) );
    return tmp;
}
