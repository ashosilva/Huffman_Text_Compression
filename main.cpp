//
//  main.cpp
//  Huffman_Text_Compression
//
//  Created by Abayomi Shosilva on 11/27/20.
//  Copyright © 2020 Abayomi Shosilva. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <bitset>
#include <vector>

using namespace std;

class OBitStream
{
public:
    OBitStream(std::ostream& output) : output_(output) {}
    
    ~OBitStream()
    {
        forceUpdate();
    }
    
    void writeBit(unsigned b)
    {
        buf_ = buf_ | ((b & 1u) << nBits_);
        if (++nBits_ == 8) {
            forceUpdate();
        }
    }
    
    void forceUpdate()
    {
        if (nBits_) {
            output_.write(reinterpret_cast<const char*>(&buf_), 1);
            buf_ = 0;
            nBits_ = 0;
        }
    }
    
private:
    std::ostream& output_;
    uint8_t buf_{0u};
    unsigned nBits_{0u};
};

class IBitStream
{
public:
    IBitStream(std::istream& input) : input_(input) {}
    
    bool readBit(unsigned& bit)
    {
        if (nBits_ == 8) {
            nBits_ = 0;
            input_.read(reinterpret_cast<char*>(&buf_), 1);
            if (!input_) {
                return false;
            }
        }
        bit = (buf_ & ( 1 << nBits_)) >> nBits_;
        ++nBits_;
        return true;
    }
    
private:
    std::istream& input_;
    uint8_t buf_{0u};
    unsigned nBits_{8u};
};


struct node {
    char character;
    int freq;
    node * left;
    node * right;
    string code;
    int created;
};


int binary_to_decimal(string&);
void Huffman(node* [], int);
void heapify(node* [], int, int);
void heapInsert(node* [], int&, node*);
void traverseCode(node*, int);
void encode(node* [], int, string, string, vector<uint8_t>&);
void decode(node* [], int, string, string, vector<uint8_t>&);
ifstream::pos_type filesize(const char* filename);


int main() {
    string in_file_name;
    string enc_file_name;
    string dec_file_name;
    
    
    struct rate {
        int og_File;
        int comp_File;
        double compRate;
    };
    
    rate table[20];
    
    for (int i = 0; i < 20; i++) {
        cout << "Input " << i+1;
        cout << endl;
        
        in_file_name = "/Users/a.b.a/Desktop/CSC_382_C++/Huffman_Text_Compression/Huffman_Text_Compression/Input_Files/";
        in_file_name += "sequence";
        in_file_name += to_string(i+1);
        in_file_name += ".txt";
       
        enc_file_name = "/Users/a.b.a/Desktop/CSC_382_C++/Huffman_Text_Compression/Huffman_Text_Compression/Encoded_Files/";
        enc_file_name += "encoded";
        enc_file_name += to_string(i+1);
        enc_file_name += ".txt";
        
        dec_file_name = "/Users/a.b.a/Desktop/CSC_382_C++/Huffman_Text_Compression/Huffman_Text_Compression/Decoded_Files/";
        dec_file_name += "decoded";
        dec_file_name += to_string(i+1);
        dec_file_name += ".txt";
        
        
        map<char, int> hist;
        ifstream in(in_file_name);
        char word;
        string test;
        while (!in.eof()) {
            getline(in, test);
            for (int i = 0; i < test.length(); i++) {
                word = test[i];
                hist[ word ]++;
            }
        }
        
        in.close();
        
        node *arr[hist.size()];
        node *B[hist.size()];
        
        int length = 0;
        typedef map<char, int>::const_iterator CI;
        for ( CI iter = hist.begin(); iter != hist.end(); ++iter ){
            //cout << iter->first << '\t' << iter->second << endl;
            arr[length] = new node;
            B[length] = arr[length];
            arr[length]->character = iter->first;
            arr[length]->freq = iter->second;
            arr[length]->left = 0;
            arr[length]->right = 0;
            arr[length]->created = 0;
            cout<< arr[length]->character << '\t' << arr[length]->freq << endl;
            length++;
        }
        
        //Takes a set of characters and their frequencies specified and creates a prefix tree.
        Huffman(arr, length);
        
        //After beign the given prefix tree, it generates the code word of the characters in the tree.
        traverseCode(arr[0], length);
        
        cout << endl;
        cout << "GENERATED HUFFMAN CODES";
        cout << endl;
        for (int i = 0; i<length; i++) {
            cout<< B[i]->character << B[i]->freq << ":" << '\t' << B[i]->code << endl;
            
        }
        
        //Part II: Implement text compression.
        
        vector<uint8_t> data;
        
        //This takes a text file name, takes the tree to get the code word, and then encodes the text file into a new file.
        encode(B, length, in_file_name, enc_file_name, data);
        
        //This takes the tree, and an encoded file, and then decodes the file.
        decode(B, length, enc_file_name, dec_file_name, data);
        
        cout << endl;
        ifstream OG(in_file_name, std::ifstream::ate | std::ifstream::binary);
        cout << "Original file size: " << OG.tellg() << " bytes";
        table[i].og_File = OG.tellg();

        cout << endl;
        ifstream COMP(enc_file_name, std::ifstream::ate | std::ifstream::binary);
        cout << "Compressed file size: " << COMP.tellg() << " bytes";
        table[i].comp_File = COMP.tellg();
        
        cout << endl;
        cout << "Compression rate: "<< ((double)COMP.tellg() / (double)OG.tellg()) * 100<< "%";
        table[i].compRate = ((double)COMP.tellg() / (double)OG.tellg()) * 100;
        cout << endl;
        cout << endl;
        
        OG.close();
        COMP.close();
    }
    
    
    cout << "HUUFMAN ALGORITHM: TEXT COMPRESSION RATE TABLE ";
    cout << endl;
    cout << setw(5) << "No." << setw(22) << "Original_File(bytes)" << setw(25) << "Compressed_File(bytes)" << setw(22) << "Compression_Rate(%)" << endl;
    for (int i =0; i < 20; i++) {
        cout << setw(4) << i+1 <<  setw(12) << table[i].og_File << setw(25) << table[i].comp_File << setw(27) << table[i].compRate << "%" << endl;
    }
    
    system("pause");
    return 0;
}


void Huffman(node* arr[], int n){
    
    node *n0, *n1, *n2;
    
    for (int i = (n/2) - 1; i >= 0; i--) {
        heapify(arr, i, n);
    }
    
    while (n-1 > 0) {
        swap(arr[0], arr[n-1]);
        n1 = arr[n-1];
        n--;
        
        heapify(arr, 0, n);

        swap(arr[0], arr[n-1]);
        n2 = arr[n-1];
        n--;
        
        heapify(arr, 0, n);
        
        n0 = new node;
        n0->character = 'X';
        n0->freq = n1->freq + n2->freq;
        n0->left = n1;
        n0->right = n2;
        n0->code = "";
        n0->created = 1;
        
        heapInsert(arr, n, n0);
        
    }
}

//HEAPIFY
void heapify(node* A[], int i, int n){
    int child = i;
    
    int l = (i*2)+1;
    int r = (i*2)+2;
    
    if (l < n && A[l]->freq < A[child]->freq){
        child = l;
    }
    if (r < n && A[r]->freq < A[child]->freq){
        child = r;
    }

    if (A[i]->freq > A[child]->freq) {
        swap(A[i], A[child]);
        heapify(A, child, n);
    }
    
}


//HEAPINSERT
void heapInsert(node* A[], int& n, node *n0){
    A[n] = n0;
    
    n++;
    int child = n-1;
    int parent = (child-1)/2;
    while(child > 0 && parent >= 0 && A[parent]->freq > A[child]->freq){
       
        swap(A[parent],A[child]);
        child = parent;
        parent = (child-1)/2;
    }
}


//HUFFMNA CODE GENERATOR
void traverseCode(node* A ,int n){
    
    if (A->right != 0 || A->left != 0) {
        A->right->code = A->code + "1";
        traverseCode(A->right, n);
        A->left->code = A->code + "0";
        traverseCode(A->left, n);
        
    }
}

//ENCODE
void encode(node* B[], int n, string in_name, string enc_name, vector<uint8_t>& data){
    
    ifstream inFile(in_name);
    ofstream outFile(enc_name);
    char word;
    
    
    OBitStream obs{outFile};
    
    string s= "";
    cout << endl;
    cout << "Message: ";
    
    string test;
    while (!inFile.eof()) {
        getline(inFile, test);
        for (int i = 0; i < test.length(); i++) {
            word = test[i];
            cout << word;
            int j = 0;
            int isit = 0;
            while (j < n && isit == 0) {
                if (word == B[j]->character) {
                    
                    s+= B[j]->code;
                    isit = 1;
                }
                j++;
            }
        }
    }
    
    
    unsigned a = 1;
    unsigned b = 0;
    
    int i = 0;
    while (i < s.length()){
        
        if (s[i] == '1') {
            obs.writeBit(a);
            data.push_back(a);
        }else{
            obs.writeBit(b);
            data.push_back(b);
        }
        i++;
    }
    obs.forceUpdate();
    
    cout << endl;
    cout << "Code: ";
    cout << s;
    
    
    
    
    inFile.close();
    outFile.close();
   
}


//DECODE
void decode(node* B[], int n, string enc_name, string dec_name, vector<uint8_t>& data){
    
    ifstream in_file(enc_name);
    
    
    ofstream dec_file(dec_name);
    
    IBitStream ibs{in_file};
    string c ="";
    
    cout << endl;
    cout << "Decoded: ";
    for(unsigned b : data) {
        unsigned bit = 0;
        if (!ibs.readBit(bit)) {
            std::cerr << "Error: failed to read" << std::endl;
        }else{
            if (bit != b) {
                std::cerr << "Error: incorrect bit" << std::endl;
            }else{
                if (bit == 1) {
                    c+= "1";
                }else{
                    c+= "0";
                }
                int i = 0;
                int isit = 0;
                while (i < n && isit == 0) {
                    if (c == B[i]->code) {
                        cout << B[i]->character;
                        dec_file << B[i]->character;
                        isit = 1;
                        c ="";
                        
                    }
                    i++;
                }
            }
        }
            
    }
    
    in_file.close();
    dec_file.close();
    
}


