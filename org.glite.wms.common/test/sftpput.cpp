#include <iostream>

#include "utilities/globus_ftp_utils.h"

using namespace std;
namespace utilities = glite::wms::common::utilities;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cout << "Usage: sftpput SRC_URL DST_URL\n" << endl;
    return -1;
  }
  else {
    utilities::globus::put(string(argv[1]), string("gsiftp://") + string(argv[2]));
  }
  return 0;
}

