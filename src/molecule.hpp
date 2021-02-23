#ifndef __MOLECULE_H__
#define __MOLECULE_H__

#include <vector>
#include <queue>
#include <utility>
#include <algorithm>
#include "atom.hpp"
#include "lattice.hpp"
#include "random.hpp"

class Molecule{
public:
    Molecule(int idx_in){idx=idx_in; idmap = VecI(4,0);}
    ~Molecule(){};

    void setid(int idx_in){this->idx = idx_in;}
    std::vector<Atom*> linkedToCluster() const; // find atoms in molecule linked to the bulk
    int countBondCur(); // count C-C bond in current configuration
    int countBondNext() const; // count C-C bond in next configuration
    bool isSurface() const; // judge if molecule has empty nearest C position
    int getRandRep() const;
    void move(); // move to next
    void putBack(); // go back to cur
    void linkedAndSurf(VecI& linked, VecI& surf); // linked: atom in cur linked to bulk; surf: empty C only linked to cur;
    VecI surf();
    void setNN(int from, int bondid, int to);
    void setNN(int from, VecI vto);
    void setmolid(){for(int i = 0; i < (int)mol->size(); i++)mol->at(i).pos = i;}
    void genidmap(int root, Atom* dest);
    bool tryFill(Atom* dest);
    bool tryAll(Atom* dest);
    // virtual bool tryFill(Atom* dest, int repidx)=0;
    
    int idx{0};
    std::vector<Atom> *mol;
    VecI idmap; // map mol bond idx to lattice bondidx. idmap[mol_bidx] = latt_bidx
    std::vector<Atom*> cur, next;
    int bondNum{0};
};
class Adamantane:public Molecule{
public:
    Adamantane(int idx_in, std::vector<Atom>* mol_);
    ~Adamantane(){};
    bool tryFill(Atom* dest, int repidx);
    static bool mol_is_set;
};
bool Adamantane::mol_is_set{false};

class Diamantane:public Molecule{
public:
    Diamantane(int idx_in, std::vector<Atom>* mol);
    ~Diamantane(){}
    static bool mol_is_set;
};
bool Diamantane::mol_is_set{false};

class Triamantane:public Molecule{
public:
    Triamantane(int idx_in, std::vector<Atom>* mol_);
    ~Triamantane(){};
    static bool mol_is_set;
};
bool Triamantane::mol_is_set{false};

class Tetramantane:public Molecule{
public:
    Tetramantane(int idx_in, std::vector<Atom>* mol_);
    ~Tetramantane(){};
    static bool mol_is_set;
};
bool Tetramantane::mol_is_set{false};

class Pentamantane:public Molecule{
public:
    Pentamantane(int idx_in, std::vector<Atom>* mol_);
    ~Pentamantane(){};
    static bool mol_is_set;
};
bool Pentamantane::mol_is_set{false};

int Molecule::countBondCur(){
    int count = 0;
    for(auto& atom:cur){
        for(auto& nn:atom->NN){
            if(nn==nullptr)continue;
            if(nn->idx!=this->idx and nn->idx!=-1) count++;
        }
    }
    bondNum = count;
    return count;
}

int Molecule::countBondNext() const {
    int count = 0;
    for(auto& atom:next){
        for(auto& nn:atom->NN){
            if(nn and nn->idx!=this->idx and nn->idx!=-1) count++;
        }
    }
    return count;
}

bool Molecule::isSurface() const {
    for(auto& atom:cur)for(auto& nn:atom->NN)if(nn==nullptr or nn->idx==-1)return true;
    return false;
}

int Molecule::getRandRep() const { return diceI(mol->size()-1);}

void Molecule::move(){
    // std::cout<<"cur:"<<cur.size()<<",next:"<<next.size()<<"\n";
    cur = std::move(next);next.clear();
    // std::cout<<"cur:"<<cur.size()<<",next:"<<next.size()<<"\n";
    assert(cur.size()==mol->size());
    for(auto& atom:cur) atom->idx = idx;
}

void Molecule::putBack(){
    for(auto& atom:cur) atom->idx = this->idx;
}

void Molecule::linkedAndSurf(VecI& linked, VecI& surf){
    linked.clear(); surf.clear();
    for(const auto& atom:cur){
        bool is_linked = false;
        for(const auto& nn:atom->NN){
            if(nn==nullptr)continue;
            if(nn->idx==-1){if(nn->countFilledNN()==1) surf.push_back(nn->pos);}
            else if (!is_linked and nn->idx!=this->idx){linked.push_back(atom->pos); is_linked=true;}
        }
    }
}

VecI Molecule::surf( ) {
    VecI res;
    for(const auto& atom:cur){
        for(const auto& nn:atom->NN){
            if(nn==nullptr)continue;
            if(nn->idx==-1){res.push_back(nn->pos);}
        }
    }
    return res;
}

std::vector<Atom*> Molecule::linkedToCluster() const {
    std::vector<Atom*> result;
    for(auto atom:cur){
        for(auto nn:atom->NN){
            if(nn and nn->idx!=this->idx and nn->idx!=-1){
                result.push_back(atom);
                break;
            }
        }
    }
    return result;
}

void Molecule::setNN(int from, int bondid, int to){
    if(to>=0)mol->at(from).NN.at(bondid) = &mol->at(to);
    else mol->at(from).NN.at(bondid) = nullptr;
}

void Molecule::setNN(int from, VecI vto){
    for(int bondid=0; bondid<4; bondid++){
        setNN(from,bondid,vto.at(bondid));
    }
}

void Molecule::genidmap(int root, Atom* dest){
    idmap = FisherYatesShuffle(4);
}

bool Molecule::tryFill(Atom* dest){
    if(dest==nullptr or dest->idx!=-1)return false;
    next.clear();
    VecI visited_idx;
    int root = getRandRep();
    genidmap(root, dest);
    std::queue<Atom*> rq,dq;
    rq.push(&mol->at(root)); dq.push(dest);
    mol->at(root).is_visited = true; visited_idx.push_back(mol->at(root).pos);
    while(!rq.empty()){
        Atom* ratom = rq.front(); rq.pop();
        Atom* datom = dq.front(); dq.pop();
        next.push_back(datom);
        for(int i = 0; i < 4; i++){
            Atom* ratom_n = ratom->NN[i];
            if(ratom_n and !ratom_n->is_visited){
                Atom* datom_n = datom->NN[idmap[i]];
                if(datom_n==nullptr or datom_n->idx!=-1){
                    unvisit(mol,visited_idx);
                    return false;
                }else{
                    ratom_n->is_visited=true; visited_idx.push_back(ratom_n->pos);
                    rq.push(ratom_n);
                    dq.push(datom_n);
                }
            }
        }
    }
    assert(next.size()==mol->size());
    unvisit(mol);
    return true;
}

bool Molecule::tryAll(Atom* dest) {
    if(dest==nullptr or dest->idx!=-1)return false;
    idmap = std::vector<int>{0,1,2,3};
    do {
        for (int root = 0; root < (int)mol->size(); ++root) {
            next.clear();
            VecI visited_idx;
            std::queue<Atom*> rq,dq;
            rq.push(&mol->at(root)); dq.push(dest);
            mol->at(root).is_visited = true; visited_idx.push_back(mol->at(root).pos);
            bool flag = true;
            while(!rq.empty()){
                Atom* ratom = rq.front(); rq.pop();
                Atom* datom = dq.front(); dq.pop();
                next.push_back(datom);
                for(int i = 0; i < 4; i++){
                    Atom* ratom_n = ratom->NN[i];
                    if(ratom_n and !ratom_n->is_visited){
                        Atom* datom_n = datom->NN[idmap[i]];
                        if(datom_n==nullptr or datom_n->idx!=-1){
                            unvisit(mol,visited_idx);
                            flag = false;
                            break;
                        }else{
                            ratom_n->is_visited=true; visited_idx.push_back(ratom_n->pos);
                            rq.push(ratom_n);
                            dq.push(datom_n);
                        }
                    }
                }
                if(!flag) break;
            }
            if(!flag) continue;
            assert(next.size()==mol->size());
            unvisit(mol);
            return true;
        }
    } while (std::next_permutation(idmap.begin(), idmap.end()));
    return false;
}

Adamantane::Adamantane(int idx_in,std::vector<Atom>* mol_):Molecule(idx_in){
    mol = mol_;
    if(!mol_is_set){
        mol->resize(10);
        setmolid();
        setNN(0,0,-1); setNN(0,1,1); setNN(0,2,2); setNN(0,3,3);
        setNN(1,0,6); setNN(1,1,0); setNN(1,2,-1); setNN(1,3,-1);
        setNN(2,0,5); setNN(2,1,-1); setNN(2,2,0); setNN(2,3,-1);
        setNN(3,0,4); setNN(3,1,-1); setNN(3,2,-1); setNN(3,3,0);
        setNN(4,0,3); setNN(4,1,7); setNN(4,2,9); setNN(4,3,-1);
        setNN(5,0,2); setNN(5,1,8); setNN(5,2,-1); setNN(5,3,9);
        setNN(6,0,1); setNN(6,1,-1); setNN(6,2,8); setNN(6,3,7);
        setNN(7,0,-1); setNN(7,1,4); setNN(7,2,-1); setNN(7,3,6);
        setNN(8,0,-1); setNN(8,1,5); setNN(8,2,6); setNN(8,3,-1);
        setNN(9,0,-1); setNN(9,1,-1); setNN(9,2,4); setNN(9,3,5);
        mol_is_set = true;
    }
}

Diamantane::Diamantane(int idx_in,std::vector<Atom>* mol_):Molecule(idx_in){
    mol = mol_;
    if(!mol_is_set){
        mol->resize(10);
        setmolid();
        setNN(0,0,-1); setNN(0,1,1); setNN(0,2,2); setNN(0,3,3);
        setNN(1,0,6); setNN(1,1,0); setNN(1,2,-1); setNN(1,3,-1);
        setNN(2,0,5); setNN(2,1,-1); setNN(2,2,0); setNN(2,3,-1);
        setNN(3,0,4); setNN(3,1,-1); setNN(3,2,-1); setNN(3,3,0);
        setNN(4,0,3); setNN(4,1,7); setNN(4,2,9); setNN(4,3,-1);
        setNN(5,0,2); setNN(5,1,8); setNN(5,2,-1); setNN(5,3,9);
        setNN(6,0,1); setNN(6,1,-1); setNN(6,2,8); setNN(6,3,7);
        setNN(7,0,-1); setNN(7,1,4); setNN(7,2,-1); setNN(7,3,6);
        setNN(8,0,-1); setNN(8,1,5); setNN(8,2,6); setNN(8,3,-1);
        setNN(9,0,-1); setNN(9,1,-1); setNN(9,2,4); setNN(9,3,5);
        mol_is_set = true;
    }
}

Triamantane::Triamantane(int idx_in,std::vector<Atom>* mol_):Molecule(idx_in){
    mol = mol_;
    if(!mol_is_set){
        mol->resize(18);
        setmolid();
        setNN(0,0,-1); setNN(0,1,-1); setNN(0,2,1); setNN(0,3,2);
        setNN(1,0,17); setNN(1,1,3); setNN(1,2,0); setNN(1,3,-1);
        setNN(2,0,16); setNN(2,1,4); setNN(2,2,-1); setNN(2,3,0);
        setNN(3,0,-1); setNN(3,1,1); setNN(3,2,5); setNN(3,3,7);
        setNN(4,0,-1); setNN(4,1,2); setNN(4,2,7); setNN(4,3,6);
        setNN(5,0,8); setNN(5,1,-1); setNN(5,2,3); setNN(5,3,-1);
        setNN(6,0,9); setNN(6,1,-1); setNN(6,2,-1); setNN(6,3,4);
        setNN(7,0,10); setNN(7,1,-1); setNN(7,2,4); setNN(7,3,3);
        setNN(8,0,5); setNN(8,1,11); setNN(8,2,-1); setNN(8,3,13);
        setNN(9,0,6); setNN(9,1,12); setNN(9,2,14); setNN(9,3,-1);
        setNN(10,0,7); setNN(10,1,15); setNN(10,2,13); setNN(10,3,14);
        setNN(11,{-1,8,17,-1});
        setNN(12,{-1,9,-1,16});
        setNN(13,{-1,-1,10,8});
        setNN(14,{-1,-1,9,10});
        setNN(15,{-1,10,16,17});
        setNN(16,{2,-1,15,12});
        setNN(17,{1,-1,11,15});
        mol_is_set = true;
    }
}

Tetramantane::Tetramantane(int idx_in,std::vector<Atom>* mol_):Molecule(idx_in){
    mol = mol_;
    if(!mol_is_set){
        mol->resize(22);
        setmolid();
        setNN(0,{-1,1,3,2});
        setNN(1,{4,0,-1,-1});
        setNN(2,{5,-1,-1,0});
        setNN(3,{6,-1,0,-1});
        setNN(4,{1,-1,8,7});
        setNN(5,{2,7,10,9});
        setNN(6,{3,8,-1,10});
        setNN(7,{11,5,-1,4});
        setNN(8,{12,6,4,-1});
        setNN(9,{13,-1,-1,5});
        setNN(10,{14,-1,5,6});
        setNN(11,{7,-1,16,15});
        setNN(12,{8,-1,-1,16});
        setNN(13,{9,15,17,-1});
        setNN(14,{10,16,-1,17});
        setNN(15,{18,13,-1,11});
        setNN(16,{19,14,11,12});
        setNN(17,{20,-1,13,14});
        setNN(18,{15,-1,21,-1});
        setNN(19,{16,-1,-1,21});
        setNN(20,{17,21,-1,-1});
        setNN(21,{-1,20,18,19});
        mol_is_set = true;
    }
}

Pentamantane::Pentamantane(int idx_in,std::vector<Atom>* mol_):Molecule(idx_in){
    mol = mol_;
    if(!mol_is_set){
        mol->resize(26);
        setmolid();
        setNN(0,{-1,1,2,3});
        setNN(1,{4,0,-1,-1});
        setNN(2,{5,-1,0,-1});
        setNN(3,{6,-1,-1,0});
        setNN(4,{1,7,8,9});
        setNN(5,{2,8,10,11});
        setNN(6,{3,9,11,12});
        setNN(7,{13,4,-1,-1});
        setNN(8,{25,5,4,-1});
        setNN(9,{17,6,-1,4});
        setNN(10,{14,-1,5,-1});
        setNN(11,{15,-1,6,5});
        setNN(12,{16,-1,-1,6});
        setNN(13,{7,-1,18,19});
        setNN(14,{10,20,-1,21});
        setNN(15,{11,22,21,23});
        setNN(16,{12,24,23,-1});
        setNN(17,{9,19,22,24});
        setNN(18,{-1,25,13,-1});
        setNN(19,{-1,17,-1,13});
        setNN(20,{-1,14,25,-1});
        setNN(21,{-1,-1,15,14});
        setNN(22,{-1,15,17,25});
        setNN(23,{-1,-1,16,15});
        setNN(24,{-1,16,-1,17});
        setNN(25,{8,18,20,22});
        mol_is_set = true;
    }
}

// bool Adamantane::tryFill(Atom* dest, int repidx){
//     VecI filledNN, emptyNN;
//     dest->classNN(filledNN, emptyNN);
//     if(repidx==0){
//         if(filledNN.size()>1) return false;
//         next.clear();
//         next.push_back(dest);
//         int i0;
//         if(!filledNN.empty()) i0 = filledNN[0];
//         else {int i = diceI(3); i0 = emptyNN[i]; emptyNN.erase(emptyNN.begin()+i);}
//         for(int i = 0; i < 3; i++){
//             int i1 = emptyNN[i];
//             int i2 = emptyNN[(i+1)%3];
//             Atom* nn1 = dest->NN[i1];
//             Atom* nn2 = nn1->NN[i0];
//             Atom* nn3 = nn2->NN[i2];
//             if(nn2->idx==-1 and nn3->idx==-1){next.push_back(nn1);next.push_back(nn2);next.push_back(nn3);}
//             else return false;
//         }
//         assert(next.size()==10);
//         return true;
//     }else{
//         int fillsize = filledNN.size();
//         if(fillsize>2) return false;
//         next.clear();
//         next.push_back(dest);
//         for(int i = 0; i<(2-fillsize); i++){
//             int j = diceI(emptyNN.size()-1);
//             filledNN.push_back(emptyNN[j]);
//             emptyNN.erase(emptyNN.begin()+j);
//         }

//         int i0 = filledNN[0], i1 = filledNN[1], i2 = emptyNN[0], i3 = emptyNN[1];
//         auto nn = dest->NN[i2]; next.push_back(nn);
//         auto nn1 = nn->NN[i0]; if(nn1->idx==-1) next.push_back(nn1); else return false;
//         auto nn2 = nn1->NN[i3]; if(nn2->idx==-1) next.push_back(nn2); else return false;
//         auto nn3 = nn2->NN[i1]; if(nn3->idx==-1) next.push_back(nn3); else return false; 
//         nn1 = nn->NN[i1]; if(nn1->idx==-1) next.push_back(nn1); else return false;
//         nn2 = nn1->NN[i3]; if(nn1->idx==-1) next.push_back(nn2); else return false;
        
//         nn = dest->NN[i3]; next.push_back(nn);
//         nn1 = nn->NN[i0]; if(nn1->idx==-1) next.push_back(nn1); else return false;
//         nn1 = nn->NN[i1]; if(nn1->idx==-1) next.push_back(nn1); else return false;
//         assert(next.size()==10);
//         return true;
//     }
// }


#endif // __MOLECULE_H__