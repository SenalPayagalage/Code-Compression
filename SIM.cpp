// On my honor, I have neither given nor received unauthorized aid on this assignment

#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <bitset>
#include <string>
#include <bits/stdc++.h>
using namespace std;

vector<bitset<32>> inv; //store original.txt file data
vector<bitset<32>> numV;
vector<int> countV;
vector<bitset<32>> finalNumV;  //dictionary with 8 entries

string inputComFile = "original.txt";
string outputComFile = "cout.txt";
string inputDecomFile = "compressed.txt";
string outputDecomFile = "dout.txt";

//For compression
void fileRead(string filename){
    string line;
    ifstream file(filename);
    while (getline(file,line)){
        inv.push_back(bitset<32>(line));
    }
    file.close();
}


void finalNumberCount(){
    for(bitset<32> num: inv){
        auto val = find(numV.begin(), numV.end(), num);
        // If element was found
        if (val != numV.end()) {
            // calculating the index of num
            int index = val - numV.begin();
            countV.at(index) += 1;
        }
        else {
            // If the element is not present in the vector
            numV.push_back(num);
            countV.push_back(1);
        }
    }

    for (int n=0; n<8; n++){
        int max = *max_element(countV.begin(), countV.end());
        auto val = find(countV.begin(), countV.end(), max);
        // If element was found
        // calculating the index of val
        int index = val - countV.begin();
        finalNumV.push_back(numV[index]);
        countV.erase(countV.begin()+index);
        numV.erase(numV.begin()+index);     
    }
}

tuple<string, int> RLECompress(int index){
    int occur = 0;
    int pos = 1;
    while (inv[index]==inv[index+pos]){
        occur+=1;
        pos++;
    }

    if (occur > 4){
        return {"00011", 4};
    }
    if (occur !=0){
        string outStr = bitset<2> (occur-1).to_string();
        outStr = "000" + outStr;
        return {outStr, occur};
    }
    else {
        return {"", 0};
    }
}


string directMatching(int index){
    auto it = find(finalNumV.begin(), finalNumV.end(), inv[index]);
    if (it != finalNumV.end()){
        int value = it - finalNumV.begin();
        string outStr = bitset<3> (value).to_string();
        outStr = "101" + outStr;
        
        return outStr;
    }
    else return "";
}

int BMlen = 4;
string genBM(string bitmask, int pos){  
    string bitMask = bitmask.substr(pos, BMlen);
    return bitMask;
}


string allMMout(int index){
    
    string MMout;
    
    for (int i=0; i<finalNumV.size(); i++){
        int MMcount =0;
        vector<int> pos;
        string lineXOR = (inv[index] ^ finalNumV[i]).to_string();
        
        for(int j = 0; j< lineXOR.size(); j++){
            if(lineXOR[j] - '0' == 1){
                MMcount ++;
                pos.push_back(j);
            }
        }
        int posRange = (pos[pos.size()-1] - pos[0]) + 1;
        //one bit mismatch
        if(MMcount == 1){
            string bitPos = bitset<5>(pos[0]).to_string();
            string dicInd = bitset<3>(i).to_string();
            MMout = "010" + bitPos + dicInd;
            return MMout;
        }
        
        //two bit mismatches
        if(MMcount == 2 && posRange==2){
            string bitPos = bitset<5>(pos[0]).to_string();
            string dicInd = bitset<3>(i).to_string();
            MMout = "011" + bitPos + dicInd;
            return MMout;
        }
        
        //bitmask based 
        if(MMcount <= 4 && posRange <= 4){
            int npos = pos[0];
            if (npos>28){
                npos = 28;
            }
            string bitPos = bitset<5>(npos).to_string();
            string bitmask = genBM( lineXOR,npos);
            string dicInd = bitset<3>(i).to_string();
            MMout = "001" + bitPos + bitmask + dicInd;
            return MMout;
        }

        //two bit mismatches anywhere
        if(MMcount == 2){
            string bitPos = bitset<5>(pos[0]).to_string();
            string bitPos_ = bitset<5>(pos[1]).to_string();
            string dicInd = bitset<3>(i).to_string();
            MMout = "100" + bitPos + bitPos_ + dicInd;
            return MMout;
        }        
    }
    return "";
}


string compress(){
    string outputStr = "";
    string compLine;
    int skip;
    for(int ind=0; ind<inv.size(); ind++){
        //checking RLE compression
        auto [compLine_, skip] = RLECompress(ind);
        if (compLine_.size()>0){
            compLine = directMatching(ind);
            if (compLine.size()>0){
                outputStr += compLine;
                outputStr += compLine_;
                ind += skip;
                continue;
            }

            compLine = allMMout(ind);
            if (compLine.size()>0){
                outputStr += compLine;
                outputStr += compLine_;
                ind += skip;
                continue;
            }
            //original binary
            compLine = "110" + inv[ind].to_string();
            outputStr += compLine;
            outputStr += compLine_;
            ind += skip;
            continue;
        }

        compLine = directMatching(ind);
        if (compLine.size()>0){
            outputStr += compLine;
            continue;
        }
        
        compLine = allMMout(ind);
        if (compLine.size()>0){
            outputStr += compLine;
            continue;
        }

        //original binary
        compLine = "110" + inv[ind].to_string();
        outputStr += compLine;
    }
    return outputStr;
}

void printFileCom(string allLines){
    ofstream compressedFile;
    compressedFile.open(outputComFile);
    for (int i=0; i<allLines.size(); i+=32){
        string singleLine = allLines.substr(i,32);
        if (singleLine.size()<32){
            singleLine.append(32-singleLine.size(),'1');
        }
        compressedFile << singleLine << endl;
    }
    
    //print 8 dictionary entries
    if (finalNumV.size()>0){
        compressedFile << "xxxx" << endl;
        for (int j=0; j<finalNumV.size(); j++){
            string com_line = finalNumV[j].to_string();
            if (j+1==finalNumV.size()){
                compressedFile << com_line;
                break;
            }
            compressedFile<< com_line << endl;
        }
    }
    compressedFile.close();
}


//For decompression
string d_allIn; //store original.txt file data
vector<bitset<32>> d_finalNumV;  //dictionary with 8 entries
string lastLine;
void d_fileRead(string filename){
    string line;
    ifstream file(filename);
    bool comDone = false;
    while (getline(file,line)){
        if (line=="xxxx"){
            comDone=true;
            continue;
        }
        if (!comDone){
            d_allIn += line;
        }
        else{
            d_finalNumV.push_back(bitset<32>(line));
        }
    }
    file.close();
}

string d_RLE(int index){
    int num = stoi(d_allIn.substr(index,2),nullptr, 2);
    string out;
    num+=1;
    for (int i=0; i<num;i++){
        out += lastLine;
    }
    return out;
}

string d_directMatching(int index){
    int num = stoi(d_allIn.substr(index,3),nullptr, 2);
    string out = d_finalNumV[num].to_string();
    lastLine = out;
    return out;
}

tuple<string, int> d_allMMout(int index, string pre){
    string out;
    if (pre=="010"){    //1 bit mismatch
        int bitPos  = stoi(d_allIn.substr(index,5),nullptr, 2);
        int dicInd = stoi(d_allIn.substr(index+5,3),nullptr, 2);
        string dicEntry = d_finalNumV[dicInd].to_string();
        if (dicEntry[bitPos]=='0'){
            dicEntry.replace(bitPos, 1, "1");
        }
        else{
            dicEntry.replace(bitPos, 1, "0");
        }
        out = dicEntry;
        lastLine = out;
        return {out, 8};
    }

    if (pre=="011"){    //2 bit mismatch
        int bitPos  = stoi(d_allIn.substr(index,5),nullptr, 2);
        int bitpos_ = bitPos +1;
        int dicInd = stoi(d_allIn.substr(index+5,3),nullptr, 2);
        string dicEntry = d_finalNumV[dicInd].to_string();

        if (dicEntry[bitPos]=='0'){
            dicEntry.replace(bitPos, 1, "1");
        }
        else{
            dicEntry.replace(bitPos, 1, "0");
        }

        if (dicEntry[bitpos_]=='0'){
            
            dicEntry.replace(bitpos_, 1, "1");
        }
        else{
            dicEntry.replace(bitpos_, 1, "0");
        }

        out = dicEntry;
        lastLine = out;
        return {out, 8};
    }

    if (pre=="001"){ //bitmask based
        int bitPos  = stoi(d_allIn.substr(index,5),nullptr, 2);
        string bMask = d_allIn.substr(index+5,4);
        int dicInd = stoi(d_allIn.substr(index+9,3),nullptr, 2);
        string dicEntry = d_finalNumV[dicInd].to_string();
        string XORchange = (bitset<4>(bMask) ^ bitset<4>(dicEntry.substr(bitPos,4))).to_string();
        dicEntry.replace(bitPos, 4, XORchange);
        out = dicEntry;
        lastLine = out;
        return {out, 12};
    }

    if (pre=="100"){    //1 bit mismatch
        int bitPos1  = stoi(d_allIn.substr(index,5),nullptr, 2);
        int bitPos2  = stoi(d_allIn.substr(index+5,5),nullptr, 2);
        int dicInd = stoi(d_allIn.substr(index+10,3),nullptr, 2);
        string dicEntry = d_finalNumV[dicInd].to_string();
        
        if (dicEntry[bitPos1]=='0'){
            dicEntry.replace(bitPos1, 1, "1");
        }
        else{
            dicEntry.replace(bitPos1, 1, "0");
        }

        if (dicEntry[bitPos2+1]=='0'){
            dicEntry.replace(bitPos2, 1, "1");
        }
        else{
            dicEntry.replace(bitPos2, 1, "0");
        }
        out = dicEntry;
        lastLine = out;
        return {out, 13};
    }
    
    if (pre=="110"){
        string lineEntry = d_allIn.substr(index,32);
        out = lineEntry;
        lastLine = out;
        return {out, 32};
    }
    return {"", 0};
}

string d_compress(){
    string d_outStr;
    int lineeee = 1;
    for (int i=0; i<d_allIn.size(); i++){
        lineeee++;
        string d_pre = d_allIn.substr(i,3);
        if (d_pre=="111"){
            return d_outStr;
        }
        if (d_pre=="000"){
            d_outStr += d_RLE(i+3);
            i+=4;
        }

        else if (d_pre=="101"){
            d_outStr += d_directMatching(i+3);
            i+=5;
        }
        
        else {
            auto [d_outStr_, step] = d_allMMout(i+3,d_pre);
            d_outStr += d_outStr_;
            i=i+2+step;
        }
    }
    return d_outStr;
}

void printFileDecom(string allLines){
    ofstream d_compressedFile;
    d_compressedFile.open(outputDecomFile);
    for (int i=0; i<allLines.size(); i+=32){
        string singleLine = allLines.substr(i,32);
        if (i+32==allLines.size()){
            d_compressedFile << singleLine;
            break;
        }
        d_compressedFile << singleLine << endl;
    }
    d_compressedFile.close();
}

int main(int argc, char *argv[]){
    if(argc==2){

        if(argv[1][0]=='1'){
            fileRead(inputComFile);
            finalNumberCount();
            string finaloutput = compress();
            printFileCom(finaloutput); 
        }
        else if(argv[1][0]=='2'){
            d_fileRead(inputDecomFile);
            string d_finaloutput = d_compress();
            printFileDecom(d_finaloutput);
        }
        else cout<<"Invalid Argument"<<endl;

    }else{
        cout<<"Argument not provided\n";
    }
    return 0;
}