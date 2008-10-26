#include "FixedLeases.h"
#include "Template.h"
#include "Log.h"
#include "SqliteDB.h"

static void log(
  const char *            module,
  const Log::MessageType  type,
  const ostringstream&    message,
  const char *            filename = 0,
  Log::MessageType        clevel   = Log::ERROR)
{        
    static Log test_log(filename,clevel,ios_base::trunc);        test_log.log(module,type,message);
};

int main(int argc, char ** argv)
{
  string   db_file="test.db";
  ostringstream oss; 
   
  oss <<"Init test";
  log("TST",Log::INFO,oss,"test.log");

  SqliteDB db(db_file,log);

  Template leases_template;
  char *   error;

  if (leases_template.parse("leases",&error)!=0)
  {
      cout << error;
      return -1;
  }

  string name="LEASE";
  vector<const Attribute*> values;

  leases_template.get(name,values);

  //Create from template
  //FixedLeases fl(&db,3,(unsigned int)17,values);

  //Read from DB
  FixedLeases fl(&db,3,(unsigned int)17);

  fl.select(&db);

  cout << fl << endl << "--------------" << endl;

  string ip, mac;

  fl.get_lease(45, ip, mac);

  cout << "Lease ip: " << ip << " mac: " << mac << endl;

  cout << "--------------" << endl << fl << endl ;

  fl.release_lease(ip);

  cout << "--------------" << endl << fl;

  return 0;
}