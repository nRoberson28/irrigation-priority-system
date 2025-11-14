#include "irrigator.h"
Region::Region() {
  m_heap = nullptr;
  m_size = 0;
  m_priorFunc = nullptr;
  m_heapType = NOTYPE;
  m_structure = NOSTRUCT;
  m_regPrior = 0;
}

Region::Region(prifn_t priFn, HEAPTYPE heapType, STRUCTURE structure, int regPrior) {
  m_heap = nullptr;
  m_size = 0;
  m_priorFunc = priFn;
  m_heapType = heapType;
  m_structure = structure;
  m_regPrior = regPrior;
}
Region::~Region() {
  clear();
}
void Region::clear() {
  clearHelper(m_heap);
  m_heap = nullptr;
  m_size = 0;
}

void Region::clearHelper(Crop* node) {
  if (node) {
    clearHelper(node->m_left);
    clearHelper(node->m_right);
    delete node;
  }
}

Region::Region(const Region& rhs) {
  m_priorFunc = rhs.m_priorFunc;
  m_heapType = rhs.m_heapType;
  m_structure = rhs.m_structure;
  m_regPrior = rhs.m_regPrior;
  m_size = rhs.m_size;
  m_heap = copyHelper(rhs.m_heap);
}

Crop* Region::copyHelper(Crop* node) {
  if (!node) return nullptr;

  Crop* newNode = new Crop(node->getCropID(), node-> getTemperature(), node->getMoisture(), node->getTime(), node->getType());
  newNode->m_npl = node->m_npl;
  newNode->m_left = copyHelper(node->m_left);
  newNode->m_right = copyHelper(node->m_right);
  return newNode;
}

Region& Region::operator=(const Region& rhs) {
  if (this != &rhs) {
    clear();
    m_priorFunc = rhs.m_priorFunc;
    m_heapType = rhs.m_heapType;
    m_structure = rhs.m_structure;
    m_regPrior = rhs.m_regPrior;
    m_size = rhs.m_size;
    m_heap = copyHelper(rhs.m_heap);
  }
  return *this;
}

Crop* Region::merge(Crop* heapOne, Crop* heapTwo) {
  if (!heapOne) return heapTwo;
  if (!heapTwo) return heapOne;

  bool heapOnePriority;
  if (m_heapType == MINHEAP)
    heapOnePriority = m_priorFunc(*heapOne) <= m_priorFunc(*heapTwo);
  else
    heapOnePriority = m_priorFunc(*heapOne) >= m_priorFunc(*heapTwo);

  if (!heapOnePriority)
    swap(heapOne, heapTwo);

  heapOne->m_right = merge(heapOne->m_right, heapTwo);

  if (m_structure == SKEW && heapOne)
    swap(heapOne->m_left, heapOne->m_right);
  else {
    int leftNpl = (heapOne->m_left) ? heapOne->m_left->m_npl : -1;
    int rightNpl = (heapOne->m_right) ? heapOne->m_right->m_npl : -1;
    if (leftNpl < rightNpl)
      swap(heapOne->m_left, heapOne->m_right);

    rightNpl = (heapOne->m_right) ? heapOne->m_right->m_npl : -1;
    heapOne->m_npl = rightNpl + 1;
  }
  return heapOne;
}

void Region::mergeWithQueue(Region& rhs) {
  if (this == &rhs) return;

  if (m_priorFunc != rhs.m_priorFunc)
    throw domain_error("Queues must have same priority function");

  if (m_structure != rhs.m_structure)
    throw domain_error("Queues must have same structure");

  m_heap = merge(m_heap, rhs.m_heap);
  m_size += rhs.m_size;

  rhs.m_heap = nullptr;
  rhs.m_size = 0;
}

bool Region::insertCrop(const Crop& crop) {
  if (!m_priorFunc) return false;

  Crop* newCrop = new Crop(crop.getCropID(), crop.getTemperature(), crop.getMoisture(), crop.getTime(), crop.getType());
  newCrop->m_left = nullptr;
  newCrop->m_right = nullptr;
  newCrop->m_npl = 0;

  m_heap = merge(m_heap, newCrop);
  m_size++;
  return true;
}

int Region::numCrops() const {
  return m_size;
}

prifn_t Region::getPriorityFn() const {
  return m_priorFunc;
}

Crop Region::getNextCrop() {
  if (!m_heap)
    throw out_of_range("Queue is empty");

  Crop result = *m_heap;
  Crop* oldRoot = m_heap;
  m_heap = merge(m_heap->m_left, m_heap->m_right);

  delete oldRoot;
  m_size--;
  return result;
}

void Region::rebuildHelper(Crop* node) {
  if (node) {
    rebuildHelper(node->m_left);
    rebuildHelper(node->m_right);

    Crop temp = *node;
    delete node;
    insertCrop(temp);
  }
}

void Region::setPriorityFn(prifn_t priFn, HEAPTYPE heapType) {
  m_priorFunc = priFn;
  m_heapType = heapType;

  if (m_heap) {
    Crop* oldHeap = m_heap;
    m_heap = nullptr;
    int oldSize = m_size;
    m_size = 0;

    rebuildHelper(oldHeap);
  }
}

void Region::setStructure(STRUCTURE structure){
  m_structure = structure;

  if (m_heap) {
    Crop* oldHeap = m_heap;
    m_heap = nullptr;
    int oldSize = m_size;
    m_size = 0;

    rebuildHelper(oldHeap);
  }
}

STRUCTURE Region::getStructure() const {
  return m_structure;
}

HEAPTYPE Region::getHeapType() const {
  return m_heapType;
}

void Region::printHelper(Crop* node) const {
  if (node) {
    cout << "(" << m_priorFunc(*node) << ")" << *node << endl;
    printHelper(node->m_left);
    printHelper(node->m_right);
  }
}

void Region::printCropsQueue() const {
  if (m_size == 0)
    cout << "Empty queue" << endl;
  else
    printHelper(m_heap);
}

void Region::dump() const {
  if (m_size == 0) {
    cout << "Empty heap.\n" ;
  } else {
    cout << "Region " << m_regPrior << ": => ";
    dump(m_heap);
  }
  cout << endl;
}

void Region::dump(Crop *pos) const {
  if ( pos != nullptr ) {
    cout << "(";
    dump(pos->m_left);
    if (m_structure == SKEW)
        cout << m_priorFunc(*pos) << ":" << pos->m_cropID;
    else
        cout << m_priorFunc(*pos) << ":" << pos->m_cropID << ":" << pos->m_npl;
    dump(pos->m_right);
    cout << ")";
  }
}

ostream& operator<<(ostream& sout, const Crop& crop) {
  sout << "Crop ID: " << crop.getCropID()
        << ", current temperature: " << crop.getTemperature()
        << ", current soil moisture: " << crop.getMoisture() << "%"
        << ", current time: " << crop.getTimeString()
        << ", plant type: " << crop.getTypeString();
  return sout;
}

//////////////////////////////////////////////////////////////
Irrigator::Irrigator(int size){
  m_capacity = size;
  m_size = 0;
  m_heap = new Region[size + 1];
}

Irrigator::~Irrigator(){
  delete[] m_heap;
}

bool Irrigator::addRegion(Region & aRegion){
  if (m_size >= m_capacity) return false;

  m_size++;
  m_heap[m_size] = aRegion;

  int current = m_size;
  int iterations = 0;

  while (current > 1) {
    int parent = current / 2;
    iterations++;

    if (m_heap[current].m_regPrior < m_heap[parent].m_regPrior) {
      swap(m_heap[current], m_heap[parent]);
      current = parent;
    }
    else
      break;
  }
  return true;
}

bool Irrigator::getRegion(Region & aRegion){
  if (m_size == 0) return false;

  aRegion = m_heap[ROOTINDEX];
  m_heap[ROOTINDEX] = m_heap[m_size];
  m_size--;
}

void Irrigator::bubbleDown(int index) {
  int current = index;

  while (current * 2 <= m_size) {
    int child = current * 2;

    if (child + 1 <= m_size && m_heap[child + 1].m_regPrior < m_heap[child].m_regPrior)
      child++;

    if (m_heap[current].m_regPrior < m_heap[child].m_regPrior) {
      swap(m_heap[current], m_heap[child]);
      current = child;
    }
    else
      break;
  }
}

void Irrigator::bubbleDownTemp(Region* heap, int size, int index) {
  int current = index;

  while (current * 2 <= m_size) {
    int child = current * 2;

    if (child + 1 <= size && heap[child + 1].m_regPrior < heap[child].m_regPrior)
      child++;

    if (heap[current].m_regPrior < heap[child].m_regPrior) {
      swap(heap[current], heap[child]);
      current = child;
    }
    else
      break;
  }
}

bool Irrigator::getNthRegion(Region & aRegion, int n){
  if (n < 1 || n > m_size) return false;

  Region* temp = new Region[m_capacity + 1];
  int tempSize = m_size;

  for (int i = 0; i < m_size; i++) {
    temp[i] = m_heap[i];
  }

  for (int i = 0; i < n - 1; i++) {
    temp[ROOTINDEX] = temp[tempSize];
    tempSize--;
    if (tempSize > 0)
      bubbleDownTemp(temp, tempSize, ROOTINDEX);
  }

  aRegion = temp[ROOTINDEX];

  for (int i = 1; i <= m_size; i++) {
    if (m_heap[i].m_regPrior == aRegion.m_regPrior) {
      m_heap[i] = m_heap[m_size];
      m_size--;
      if (i <= m_size) {
        int parent = i / 2;
        if (parent >= 1 && m_heap[i].m_regPrior < m_heap[parent].m_regPrior) {
          while (i > 1 && m_heap[i].m_regPrior < m_heap[parent].m_regPrior) {
            swap(m_heap[i], m_heap[parent]);
            i = parent;
          }
        }
        else
          bubbleDown(i);
      }
      break;
    }
  }
  delete[] temp;
  return true;
}

void Irrigator::dump(){
    dump(ROOTINDEX);
    cout << endl;
}

void Irrigator::dump(int index){
  if (index <= m_size){
    cout << "(";
    dump(index*2);
    cout << m_heap[index].m_regPrior;
    dump(index*2 + 1);
    cout << ")";
  }
}

bool Irrigator::setPriorityFn(prifn_t priFn, HEAPTYPE heapType, int n){
  if (n < 1 || n > m_size) return false;

  Region temp;
  if (!getNthRegion(temp, n)) return false;

  temp.setPriorityFn(priFn, heapType);
  return addRegion(temp);
}

bool Irrigator::setStructure(STRUCTURE structure, int n){
  if (n < 1 || n > m_size) return false;

  Region temp;
  if (!getNthRegion(temp, n)) return false;

  temp.setStructure(structure);
  return addRegion(temp);
}

bool Irrigator::getCrop(Crop & aCrop){
  if (m_size == 0) return false;

  while (m_size > 0) {
    if (m_heap[ROOTINDEX].numCrops() > 0) {
      aCrop = m_heap[ROOTINDEX].getNextCrop();
      return true;
    }

    m_heap[ROOTINDEX] = m_heap[m_size];
    m_size--;

    if (m_size > 0)
      bubbleDown(ROOTINDEX);
  }
  return false;
}