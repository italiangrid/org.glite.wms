#include <sstream>
#include <iostream>
#include "glite/wms/checkpointing/checkpointing.h"

using namespace std ;


std::string itoa(const int& x)
{
  std::ostringstream o;
  o << x;
  return o.str();
}

int istep;
string lstep;


int main() {

glite::wms::checkpointing::JobState* statePtr;

// Initialize state
try {
statePtr = new glite::wms::checkpointing::JobState(glite::wms::checkpointing::JobState::job);
} catch (glite::wms::checkpointing::ChkptException e) {
cerr << "Error: failed to initialize JobState object. " << endl;
return 1;
}

// get current step and try to read value of an_int
 glite::wms::checkpointing::Step st = statePtr->getCurrentStep();

 if (st.isInteger()) cout << "Job started at step " << st.getInteger() << endl;
 if (st.isLabel()) cout << "Job started at step " << st.getLabel() << endl;

for(int i=1; i<10; i++){
 
 if (st.isInteger()){
  istep=st.getInteger(); 
  lstep=itoa(istep);
 };
 if (st.isLabel()){
  lstep=st.getLabel();
 };
 cout << "Job get step " << lstep << endl;
 string nam="item"+itoa(i);
 statePtr->saveValue(nam,lstep);
 statePtr->saveState();

 try { 
  st = statePtr->getNextStep();
 } catch (glite::wms::checkpointing::ChkptException e) {
  cout << "Last step" << endl;
  return 0;
 }

// statePtr->saveValue("value2",102);
// statePtr->saveState();

}
}

