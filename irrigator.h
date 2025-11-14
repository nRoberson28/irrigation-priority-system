#ifndef IRRIGATOR_H
#define IRRIGATOR_H
#include <stdexcept>
#include <iostream>
#include <string>
using namespace std;
class Tester;   // forward declaration (for testing purposes)
class Irrigator;   // forward declaration
class Region;   // forward declaration
class Crop;     // forward declaration

// Constant parameters, min and max values
#define ROOTINDEX 1
#define DEFAULTCROPID 100000
const int MINCROPID = 100001;// minimum crop ID
const int MAXCROPID = 999999;// maximum crop ID
// The temperature
const int MINTEMP = 30;     // lowest priority
const int MAXTEMP = 110;    // highest priority
// The soil moisture percentage
const int MINMOISTURE = 1;  // highest priority
const int MAXMOISTURE = 100;// lowest priority
// time of day, MORNING is the highest priority
enum TIME {MORNING,NOON,AFTERNOON,NIGHT};
const int MINTIME = MORNING;// highest priority
const int MAXTIME = NIGHT;  // lowest priority
// The type of plant based on the water requirements
enum PLANT {BEAN,MELON,MAIZE,SUNFLOWER,COTTON,CITRUS,SUGARCANE};
const int MINTYPE = BEAN;       // lowest priority
const int MAXTYPE = SUGARCANE;  // highest priority

enum HEAPTYPE {MINHEAP, MAXHEAP, NOTYPE};
enum STRUCTURE {SKEW, LEFTIST, NOSTRUCT};

// Priority function pointer type
typedef int (*prifn_t)(const Crop&);

class Crop{
    public:
    friend class Tester; // for testing purposes
    friend class Region;
    Crop(){
        m_cropID = DEFAULTCROPID;m_temperature = MINTEMP;
        m_moisture = MAXMOISTURE;m_time = MAXTIME;m_type = MINTYPE;
        m_right = nullptr;
        m_left = nullptr;
        m_npl = 0;
    }
    Crop(int ID, int temperature, int moisture, int time, int type){
        if (ID < MINCROPID || ID > MAXCROPID) m_cropID = DEFAULTCROPID;
        else m_cropID = ID;
        if (temperature < MINTEMP || temperature > MAXTEMP) m_temperature = MINTEMP;
        else m_temperature = temperature;
        if (moisture < MINMOISTURE || moisture > MAXMOISTURE) m_moisture = MAXMOISTURE;
        else m_moisture = moisture;
        if (time < MINTIME || time > MAXTIME) m_time = MAXTIME;
        else m_time = time;
        if (type < MINTYPE || type > MAXTYPE) m_type = MINTYPE;
        else m_type = type;
        m_right = nullptr;
        m_left = nullptr;
        m_npl = 0;
    }
    int getCropID() const {return m_cropID;}
    int getTemperature() const {return m_temperature;}
    int getMoisture() const {return m_moisture;}
    int getTime() const {return m_time;}
    string getTimeString() const {
        string result = "UNKNOWN";
        switch (m_time)
        {
        case MORNING: result = "MORNING"; break;
        case NOON: result = "NOON"; break;
        case AFTERNOON: result = "AFTERNOON"; break;
        case NIGHT: result = "NIGHT"; break;
        default: break;
        }
        return result;
    }
    int getType() const {return m_type;}
    string getTypeString() const {
        string result = "UNKNOWN";
        switch (m_type)
        {
        case BEAN: result = "BEAN"; break;
        case MELON: result = "MELON"; break;
        case MAIZE: result = "MAIZE"; break;
        case SUNFLOWER: result = "SUNFLOWER"; break;
        case COTTON: result = "COTTON"; break;
        case CITRUS: result = "CITRUS"; break;
        case SUGARCANE: result = "SUGARCANE"; break;
        default: break;
        }
        return result;
    }

    // Overloaded insertion operators for Crop
    friend ostream& operator<<(ostream& sout, const Crop& crop);

    private:
    int m_cropID;       // every crop is identified by a unique ID
    // m_temperature shows the temperature at the calculation time
    // the lower the temperature is the lower the priority is
    int m_temperature;  // 30-110 degree Fahrenheit
    // m_moisture shows how moist is the soil for the crop object at the calculation time
    // a value of 1 indicates the highest priority
    // a value of 100 indicates the lowest proprity
    int m_moisture;     // 1-100 %
    // m_time shows the time of the say at the calculation time
    // the time of day is divided into 4 windows
    // a value of 0 means a higher priority
    // a value of 3 means a lower priority
    int m_time;         // 0-3, an enum type is defined for this
    // m_type shows the type of a crop based on the plant watering requirement
    // a value of 0 means a lower priority,
    // a value of 6 means a higher priority
    int m_type;         // 0-6, an enum type is defined for this

    Crop * m_right;   // right child
    Crop * m_left;    // left child
    int m_npl;        // null path length for leftist heap
};

class Region{
    public:
    friend class Tester; // for testing purposes
    friend class Irrigator;
    Region();
    Region(prifn_t priFn, HEAPTYPE heapType, STRUCTURE structure, int regPrior);
    ~Region();
    Region(const Region& rhs);
    Region& operator=(const Region& rhs);
    bool insertCrop(const Crop& crop);
    Crop getNextCrop(); // Return the highest priority crop
    void mergeWithQueue(Region& rhs);
    void clear();
    int numCrops() const; // Return number of nodes in queue
    void printCropsQueue() const; // Print the queue using preorder traversal
    prifn_t getPriorityFn() const;
    // Set a new priority function.
    void setPriorityFn(prifn_t priFn, HEAPTYPE heapType);
    HEAPTYPE getHeapType() const;
    STRUCTURE getStructure() const;
    // Set a new data structure (skew/leftist).
    void setStructure(STRUCTURE structure);
    void dump() const; // For debugging purposes

    private:
    Crop * m_heap;          // Pointer to root of the heap
    int m_size;             // Current size of the heap
    prifn_t m_priorFunc;    // Function to compute priority
    HEAPTYPE m_heapType;    // either a MINHEAP or a MAXHEAP
    STRUCTURE m_structure;  // skew heap or leftist heap
    int m_regPrior;         // this holds the priority of the region

    void dump(Crop *pos) const; // helper function for dump

    void clearHelper(Crop* node);
    Crop* copyHelper(Crop* node);
    Crop* merge(Crop* heapOne, Crop* heapTwo);
    void rebuildHelper(Crop* node);
    void printHelper(Crop* node) const;

};

class Irrigator{
    public:
    friend class Tester; // for testing purposes
    Irrigator(int size);
    ~Irrigator();
    bool addRegion(Region & aRegion); // enqueue function
    bool getRegion(Region & aRegion); // dequeue function
    bool getCrop(Crop & aCrop);
    bool getNthRegion(Region & aRegion, int n);
    void dump(); // For debugging purposes
    // change priority function for the Nth highest priority region
    bool setPriorityFn(prifn_t priFn, HEAPTYPE heapType, int n);
    // change structure for the Nth highest  priority region
    bool setStructure(STRUCTURE structure, int n);

    private:
    Region * m_heap;          // Array to hold the heap
    int m_capacity;           // size of array
    int m_size;               // Current size of the heap

    void bubbleDown(int index);
    void bubbleDownTemp(Region* heap, int size, int index);

    void dump(int index);
};
#endif