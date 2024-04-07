#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <deque>

#include <time.h> //for simple random number generator seeding
#include <stdlib.h>

using namespace std;

//A recreation of Mark V. Shaney chatbot, based the original pre-K&R C-89 code here (https://9p.io/cm/cs/pearls/markov.c) but with C++ conveniences

//We create a hash table that maps each individual word in the training text to a list of words that have immediately followed it whenever it occurs in the training text.
//The hash table is the unordered_map<string, deque>.  The string is the keyword, the deque is the list of words which have followed the keyword at any time in the training text.
//We work through the training text word by word.  For every word we encounter, we look it up in the unordered_map, and push the word that follows it to the corresponding deque.
//Thus the deque may contain the same word multiple times.  E.g., {cat, {sleeps, meows, sits, eats, meows, sleeps}}
//The frequency with which a given word appears in the deque will exactly correspond to the frequency that it follows the keyword in the training text.
//We could use less memory by insisting on uniqueness in the deque and saving a word-frequency pair for every keyword to capture the frequency data information instead but this way is simpler.

//When we want to generate new text, we prime the dialog with a word (or we could choose a starting word randomly from the vocabulary.)
//We look up the last word in the conversation in the unordered_map, which returns the deque of words which have followed the keyword in the trainint text.
//We then choose randomly one of the words from the deque and add it as the new last word.
//Because the frequency that a word occurs in the deque corresponds to the frequency with which it follows the keyword in the training text, choosing the word randomly mimics the usage of the training text.
//Thus picking the word randomly from the deque corresponds to a Markov chain linking the keywords to each other.

int main()
{

    //Load the training text(s) here
    ifstream input_file("WholeSumma.txt");
    if (!input_file.is_open()) { cerr<<"Error opening input file."<<endl; return -1; }

    deque<string> EmptyDeque;
    unordered_map<string, deque<string>> WordRelsMap;

    string token;
    int counter=0;  //Optional, just to count total tokens ingested
    size_t largest=0;  //Optional, just to check tokenization by looking for the largest token
    while (input_file>>token) {
        if (token.length()>largest){
                largest=token.length();
                //cout<<token<<endl;  //Optionally prints the largest token each time a larger one is found
        }
        counter++;
        WordRelsMap[token]; //If the keyword does not exist, this adds a keyword entry to our map, with an empty deque as its initial value.  Otherwise, does nothing.  We will build the deque for each keyword in a second pass.
    }

    cout<<"Processed "<<counter<<" tokens, largest was "<<largest<<endl;
    cout<<"Size of map="<<WordRelsMap.size()<<endl;

    cout<<"Populating Word relations map."<<endl;

    input_file.clear();  //Reset input file stream to beginning, clearing any flags
    input_file.seekg(0,ios::beg);

    string prevToken, answer;
    input_file>>prevToken;

    while(input_file>>token) {  //For every word in the training text
        WordRelsMap[prevToken].push_back(token);  //Look it up in the map, and push the word that follows it to the corresponding deque for that keyword
        prevToken=token;
    }

    input_file.close();

    string userInput;  //Get some starting word.  Here we let the user decide.  You can enter a phrase but it only looks at the last word.
    deque<string> tokenizedInput;  //This deque keeps track of the state of the generated text.  The new word is pushed to the back, and the front popped.

    ofstream output_file("output.txt",ios::app);  //Copy the output to a text file
    if (!output_file.is_open()) { cerr<<"Error opening output file."<<endl; return -1;}

    bool keepTalking=true;
    while (keepTalking) {
        cout<<"Start talking."<<endl;
        getline(std::cin, userInput);

        std::istringstream iss(userInput);
        while (iss>>token) {
            tokenizedInput.push_back(token);  //Loads the user's starting phrase into the deque which contains last few words of the conversation
        }

        size_t dequeLength=0;
        srand((unsigned int)time(NULL));
        int rv;

        //Start off with a final word that is in the WordRelsMap vocabulary.  If not, backup one more word in the starting phrase and use that instead
        while (WordRelsMap.find(tokenizedInput.back())==WordRelsMap.end()) {
            tokenizedInput.pop_back();
            if (tokenizedInput.size()==0) {
                cout<<"Bad starting phrase, retry."<<endl;  //If no word in the user's starting phrase is in the vocabulary, ask for a new starting word
                getline(std::cin, userInput);
                while (iss>>token) {
                    tokenizedInput.push_back(token);
                }
            }
        }  //OK, now we've got a starting word in our vocabulary.  Start generating new text.

        output_file<<"\n\n"<<tokenizedInput.back(); //Write our first word to the output file
        counter=0;
        while (counter<300) {  //Set how many words you would like generated at a time here
            //Look up the last word in our conversation deque, find it in WordRelMap.  It should always be found. It returns a key/value pair, so ->second return the deque of words which follow the keyword in the training text.  Get the size of this deque.
            dequeLength=WordRelsMap.find(tokenizedInput.back())->second.size();
            rv = rand()%dequeLength;  //Choose a random value from 0 to dequeLength-1.
            token=(WordRelsMap.find(tokenizedInput.back())->second)[rv]; //Pick that word from the deque as the next word in our generated text
            cout<<" "<<token<<std::flush;
            output_file<<" "<<token<<std::flush;
            tokenizedInput.pop_front();
            tokenizedInput.push_back(token);
            counter++;
        }

    cout<<"\n\nGenerate again? (Y/N)"<<endl;
    cin>>answer;
    if ((answer=="Y")||(answer=="y")||(answer=="YES")) keepTalking=true; else keepTalking=false;
    } //End-while keeptaling

    output_file.close();

}  //End of main
