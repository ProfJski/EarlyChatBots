#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "boost/tokenizer.hpp"
#include "boost/random.hpp"
#include <map>
#include <utility> //for make_pair
#include "boost/algorithm/string/case_conv.hpp"
#include <deque>
#include <time.h> //For seeding random number generator

using namespace std;

// Dissociated Press: a simple 1970s chatbot, implemented in C++.  https://en.wikipedia.org/wiki/Dissociated_press
//
// We select a word frame size which will control the algorithm.  Word frames of 3-5 words work best.  You can try a frame size of 1 or 2 but the result will be more "dissociated" or jibberish.
// Too large a frame size will lead to looping, where the same text is repeated over and over because it can't find other matches (see below).
// The algorithm: First, we populate the word frame of size X with X words in a row taken from a random location in the source text.
// Then we search the source text for another instance of those X words in a row.  When found, we add the word which follows the match to the end of the word frame, print it to the screen,
// and drop the first word of the frame (giving us a new frame of size X again: ABC --> BCD --> CDE as the algorithm progresses and new matches are fouind) Repeat as long as desired.


//boost::random::mt19937 mt(random_device{}()); //This gives a better random init if you have a random device, otherwise use the following line which seeds random from timer
boost::random::mt19937 mt(time(NULL));
boost::random::uniform_real_distribution<float> dist(0.0,1.0);
boost::random::uniform_int_distribution<size_t> zeroToFour(0,4);
boost::random::uniform_int_distribution<size_t> zeroTo511(0,511);

int main()
{

    //Open the training text and tokenize it into big vector<string> of words
    ifstream input_file("WholeSumma.txt");
    if (!input_file.is_open()) {
        cerr<<"Could not open file."<<endl;
        return -1;
    }

    boost::char_separator<char> sep(" \t\n\r\f,.:;?!@#$%^&*[]{}()<>_\"'"); //Will split up text into tokens based on any of these characters
    typedef boost::tokenizer<boost::char_separator<char>> TOKENIZER_TYPE;

    vector<string> TextWords; //We are storing all our tokenized text data in this vector for easy retrieval
    string line;
    string temp;
    while (getline(input_file,line)) {
        TOKENIZER_TYPE tokens(line,sep);
        for (const auto &w: tokens) {
            temp=w;
            boost::algorithm::to_lower(temp); //Make everything lowercase so it matches the format of our dictionary
            TextWords.push_back(temp);
        }
    }

    cout<<"Stored in TextWords "<<TextWords.size()<<endl;

    input_file.close();

/*
    //If you want to load more files do a similar thing....
    input_file.open("MoreTextsToTrain.txt");
    if (!input_file.is_open()) {cerr<<"Error opening file."<<endl; return -1;}
    while (getline(input_file,line)) {
        TOKENIZER_TYPE tokens(line,sep);
        for (const auto &w: tokens) {
            temp=w;
            boost::algorithm::to_lower(temp);
            TextWords.push_back(temp);
        }
    }

    cout<<"Stored in TextWords "<<TextWords.size()<<endl;

    input_file.close();
*/

//If you want to see how the first few thousands tokens turned out, uncomment below.
/*
    for (size_t i=0;i<3000;i++) {
        cout<<TextWords[i]<<" ";
    }
    cout<<endl;
*/

//Initialize starting string

    size_t FRAME_SIZE=4;
    bool acceptable=false;
    string answer;
    deque<string> wordFrame;

    while (acceptable==false) {
        cout<<"Starting string...";
        size_t randoStartPos=(TextWords.size()-FRAME_SIZE)*dist(mt);  //Pick a random location anywhere in the text, no closer than FRAME_SIZE words from the end
        cout<<"starting at "<<randoStartPos<<" "<<endl;
        for (size_t i=0;i<FRAME_SIZE;i++) {
            wordFrame.push_back(TextWords[randoStartPos+i]);
        }
        for (size_t i=0;i<FRAME_SIZE;i++) {
            cout<<wordFrame[i]<<" ";  //Print our starting word frame and see if the user likes it
        }
        cout<<endl;

        cout<<"\n Acceptable?"<<endl;
        cin>>answer;
        if (answer=="Y") {
                acceptable=true;
            }
        else {
            wordFrame.clear();
        }
    }

// Start talking

    bool keepTalking=true;
    bool matched=false;
    size_t firstWordInstance=0;
    size_t wrapAroundCounter=0;
    size_t TextSize=TextWords.size();
    size_t wordcounter=0;

    ofstream convo("convo.txt", ios_base::app);
    if (!convo.is_open()) {
        cerr<<"Could not open convo.txt file."<<endl;
        return -1;
    }

    cout<<"/n-----/nNew Conversation.  Frame size="<<FRAME_SIZE<<endl;
    convo<<"/n-----/nNew Conversation.  Frame size="<<FRAME_SIZE<<endl;
    for (size_t i=0;i<FRAME_SIZE;i++) {
            cout<<wordFrame[i]<<" ";
            convo<<wordFrame[i]<<" ";
        }
    cout<<flush;
    convo<<flush;

    while(keepTalking && (wordcounter<300) ) {
        wordcounter++;
        wrapAroundCounter=0;
        firstWordInstance=TextSize*dist(mt); //start searching for a match at a random location in the larger text, and count increments separately to see when we've wrapped around
        while (!matched) {
            matched=true;
            for (size_t i=0;i<FRAME_SIZE;i++) {  //Check each word in the frame, starting with the first, to see if we match our position in the source text
                matched=matched&&(TextWords[(firstWordInstance+i)%TextSize]==wordFrame[i]); // We AND match with itself and the result of checking the next word so that matched==true only if all words in the frame match the source text
            }                                                                               // The index of Textwords is modded with %TextSize so 1) we don't access the vector out-of-bounds if searching near the end, and 2) we effectively wrap from the last word of the text to the beginning if matching near the end
            if (matched) {  //If so, pop the first word off the frame, add the next word that follows from the source text, print it, reset wrap-around counter and matched variable
                wordFrame.pop_front();
                wordFrame.push_back(TextWords[(firstWordInstance+FRAME_SIZE)%TextSize]);
                /* cout<<"New Frame"<<endl;
                for (size_t x=0;x<FRAME_SIZE;x++) {
                    cout<<wordFrame[x];
                }
                */
                cout<<wordFrame.back()<<" "<<flush;
                convo<<wordFrame.back()<<" "<<flush;
                firstWordInstance=TextSize*dist(mt); //reset search position for new frame
                wrapAroundCounter=0;
                matched=false;
            } //end if-matched
            else {  //If not, update counters and compare with the next word of the source text
                firstWordInstance++;
                wrapAroundCounter++;
                if (wrapAroundCounter>=TextSize-(FRAME_SIZE+1)) {  //If we find no match after looking through the whole source text, terminate.
                    cout<<"No match found.  Terminating."<<endl;
                    convo<<"No match found.  Terminating."<<endl;
                    keepTalking=false;
                    matched=true;
                    break;
                }
            }

        }
    }

    convo.close();


return 0;
}

//An earlier simple search routine, which searches for the first instance of the wordFrame in the larger text starting from the beginning every time, rather than
//starting from a random location in the source text.  It works, but it tends to loop.  Sooner or later it finds earlier and earlier matches in the source text
//until it hits on some distinctive phrase that occurs early in the text and keeps looping on that, since nothing else matches.

/*
// Start talking

    bool keepTalking=true;
    bool matched=false;
    size_t firstWordInstance=0;

    cout<<"Conversation..."<<endl;
    for (size_t i=0;i<FRAME_SIZE;i++) {
            cout<<wordFrame[i]<<" ";
        }
    cout<<flush;

    while(keepTalking) {
        firstWordInstance=0;
        while (!matched) {
            matched=true;
            for (size_t i=0;i<FRAME_SIZE;i++) {
                matched=matched&&(TextWords[firstWordInstance+i]==wordFrame[i]);
            }
            if (matched) {
                wordFrame.pop_front();
                wordFrame.push_back(TextWords[firstWordInstance+FRAME_SIZE]);
                // cout<<"New Frame"<<endl;
                //for (size_t x=0;x<FRAME_SIZE;x++) {
                //    cout<<wordFrame[x];
                //}
                cout<<wordFrame.back()<<" "<<flush;
                firstWordInstance=0; //reset search position for new frame
                matched=false;
            } //end if-matched
            else {
                firstWordInstance++;
                if (firstWordInstance>=TextWords.size()-(FRAME_SIZE+1)) {
                    cout<<"No match found.  Terminating."<<endl;
                    keepTalking=false;
                    matched=true;
                    break;
                }
            }

        }
    }

*/
