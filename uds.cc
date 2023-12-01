/*
 Up-(side)-Down Simplification v2.
 SoCG 2024 Edition
 
 Notes:
 - You will need the Regina runtime libraries and dependencies to build this.
 - This is a work-in-progress and is subject to change.
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string>
#include <cstring>
#include <vector>
#include "triangulation/dim4.h"
#include "utilities/randutils.h"

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << progName << " { Isomorphism signature }" <<
    " [ -h=height] [ -w=width ] [ -e=epochs ] [ -r ] \n\n";
    std::cerr << "    -h : Maximum number of 2-4 moves to perform (height) " <<
    "(default 10).\n";
    std::cerr << "    -w : Maximum number of 3-3 and 4-4 moves to perform (width) "
    << "(default 30). \n";
    std::cerr << "    -e : Number of epochs (default 10). \n";
    std::cerr << "    -r : Use random choices. \n";
    std::cerr << std::endl;
    std::cerr << "Example usage: " << std::endl;
    std::cerr << progName << " mLvAwAQAPQQcfffhijgjgjkkklklllaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -h2 -w15 -e5 -r \n";
    exit(1);
}

bool ap(char arr[], char c) {
    return arr[0] == '-' && arr[1] == c;
}

regina::Triangulation<4> down(regina::Triangulation<4> T) {
    
    std::vector<regina::Edge<4>*> edge20Vec;
    std::vector<regina::Triangle<4>*> triangle20Vec;

    bool tryEdgeDescent = true;
    bool tryTriangleDescent = true;
    
    while (tryEdgeDescent) {
        
        edge20Vec.clear();
       
        for (regina::Edge<4>* edge : T.edges()) {
            if (T.twoZeroMove(edge, true, false)) {
                edge20Vec.emplace_back(edge);
            }
        }
    
        for (regina::Edge<4>* edge : edge20Vec) {
            T.twoZeroMove(edge, false, true);
            break;
        }

        if (edge20Vec.empty()) { 
            tryEdgeDescent = false;
            break;
        }

    }
    
    while (tryTriangleDescent) {

        triangle20Vec.clear();
        
        for (regina::Triangle<4>* triangle : T.triangles()) {
            if (T.twoZeroMove(triangle, true, false)) {
                triangle20Vec.emplace_back(triangle);
            }
        }
         
        for (regina::Triangle<4>* triangle : triangle20Vec) {
            T.twoZeroMove(triangle, false, true);
            break;
        }

        if (triangle20Vec.empty()) {
            tryTriangleDescent = false;
            break;
        }
    }

    return T;
}

regina::Triangulation<4> threeThreeDown(regina::Triangulation<4> T, int cap, bool rnd) {
    
    int threeThreeAttempts = 0;
   
    unsigned long ttas;

    std::vector <regina::Triangle<4>*> threeThreeAvailable;
   
    regina::Triangle<4>* threeThreeChoice;
    
    while (true) {
        if (threeThreeAttempts >= cap) {
            break;
        }

        threeThreeAvailable.clear();
        for (regina::Triangle<4>* tri : T.triangles()) {
            if (T.pachner(tri, true, false)) {
                threeThreeAvailable.emplace_back(tri);
            }
        }
    
        if (threeThreeAvailable.empty()) {
            break;
        }

        ttas = threeThreeAvailable.size();
         
        if (rnd) {
            threeThreeChoice =
            threeThreeAvailable[rand()%ttas];
        }
        else {
            threeThreeChoice = threeThreeAvailable[threeThreeAttempts%ttas];
        }

        T.pachner(threeThreeChoice, false, true);
        
        T = down(T);

        threeThreeAttempts++;
    }

    return T;
}

regina::Triangulation<4> fourFourDown(regina::Triangulation<4> T, int cap, bool rnd) {
    
    int fourFourAttempts = 0;
    
    unsigned long ffas;
    
    std::vector <regina::Edge<4>*> fourFourAvailable;
    
    regina::Edge<4>* fourFourChoice;
  
    while (true) {
        if (fourFourAttempts >= cap) {
            break;
        }

        fourFourAvailable.clear();
        for (regina::Edge<4>* e : T.edges()) {
            if (T.fourFourMove(e, true, false)) {
                fourFourAvailable.emplace_back(e);
            }
        }
        
        if (fourFourAvailable.empty()) {
            break;
        }

        ffas = fourFourAvailable.size();
         
        if (rnd) {
            fourFourChoice =
            fourFourAvailable[rand()%ffas];
        }
        else {
            fourFourChoice = fourFourAvailable[fourFourAttempts%ffas];
        }

        T.fourFourMove(fourFourChoice, false, true);
        T = down(T);

        fourFourAttempts++;
    }

    return T;
}

int main(int argc, char* argv[]) {
    int twoFourCap = 10;
    int widthCap = 30;
    int epochs = 10;
    bool useRandom = false;
    long rndSeed;

    if (argc < 2) {
        usage(argv[0], "Error: No isomorphism signature provided.");
    }
    if (2 < argc) {
        for (int i=2; i<argc; ++i) {
            if (ap(argv[i],'h')) {
                twoFourCap = std::stoi(argv[i]+=2);
            }
            else if (ap(argv[i],'w')) {
                widthCap = std::stoi(argv[i]+=2);
            }
            else if (ap(argv[i],'e')) {
                epochs = std::stoi(argv[i]+=2);
            } 
            else if (!strcmp(argv[i],"-r")) {
                useRandom = true;
            }
            else {
                usage(argv[0], std::string("Invalid option: ")+argv[i]);
            }
        }
    }

    if (useRandom) {
        rndSeed = (long)time(0);
        std::cerr << "Random seed: " << rndSeed << std::endl;
        srand(rndSeed);
    }

    std::string initSig, newSig;
    initSig = newSig = argv[1];

    int currentEpoch = 1;

    regina::Triangulation<4> working =
    regina::Triangulation<4>::fromIsoSig(initSig);

    size_t currentEdges, originalEdges;
    originalEdges = currentEdges = working.countEdges();

    int attempts; 
    clock_t time;
    double timeTaken;

    int twoFourAttempts = 0;
    unsigned long tfas;
    std::vector <regina::Tetrahedron<4>*> twoFourAvailable;
    regina::Tetrahedron<4>* twoFourChoice;

    while (currentEpoch < epochs) {
       
        working = regina::Triangulation<4>::fromIsoSig(newSig);
        //currentEdges = working.countEdges();

        time = clock();
    
        working = down(working); 
        currentEdges = working.countEdges();
        working = threeThreeDown(working,widthCap,useRandom);
        working = fourFourDown(working,widthCap,useRandom);

//        working = up(working,twoFourCap,useRandom);
    
        twoFourAttempts = 0;
        while (twoFourAttempts < twoFourCap) {
            for (int i=0; i<twoFourAttempts; i++) {
                twoFourAvailable.clear();
                for (regina::Tetrahedron<4>* tet : working.tetrahedra()) {
                    if (working.pachner(tet, true, false)) {
                        twoFourAvailable.emplace_back(tet);
                    }
                }

                tfas = twoFourAvailable.size();
         
            
                if (useRandom) {
                    twoFourChoice =
                    twoFourAvailable[rand()%tfas];
                }
                else {
                    twoFourChoice = twoFourAvailable[i%tfas];
                }

                working.pachner(twoFourChoice, false, true);
            }

            working = threeThreeDown(working,widthCap,useRandom);
            working = fourFourDown(working,widthCap,useRandom);

            twoFourAttempts++;
        }
        
        time = clock() - time;
        timeTaken = ((double)time)/CLOCKS_PER_SEC; 

        newSig = working.isoSig();
        
        if (working.countEdges() < originalEdges) {
        std::cout << newSig << std::endl;
        std::cout << currentEpoch << " | " << working.countVertices() <<
        ", " << currentEdges << " -> " << working.countEdges() << ", " <<
        working.size() << " | " << timeTaken << std::endl;
        }

        currentEpoch+=1;    
    }

    working = down(working);
    working = threeThreeDown(working,widthCap,useRandom); 
    working = fourFourDown(working,widthCap,useRandom);

    std::cout << "\007";

    std::cout << working.isoSig() << std::endl;
    std::cout << "End | " << working.countVertices() << ", " <<
    originalEdges << " -> " << working.countEdges() << ", " <<
    working.size() << std::endl; 

    return 0;
}
