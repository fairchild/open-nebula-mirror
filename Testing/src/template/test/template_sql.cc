#include "TemplateSQL.h"
#include "SqliteDB.h"
#include "Log.h"

#include <string>
#include <iostream>
#include <unistd.h>

#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <TestCaller.h>
#include <ui/text/TestRunner.h>

using namespace std;

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* --------------------------------------- */
/*   WRAPPER TO ACCESS TEMPLATESQL CLASS   */
/* --------------------------------------- */

class TSQL : public TemplateSQL
{
public:
    TSQL(const char * _table,
         int          template_id = -1,
         bool         replace     = false,
         const char   separator   = '=',
         const char * xml_root    = "TEMPLATE"):
        TemplateSQL(_table,template_id,replace,separator,xml_root){}

    ~TSQL(){}
    
    /* --------------------------------------------------------------------- */
    int insert(SqliteDB * db){return TemplateSQL::insert(db);}
    int update(SqliteDB * db){return TemplateSQL::update(db);}
    int select(SqliteDB * db){return TemplateSQL::select(db);}
    int drop  (SqliteDB * db){return TemplateSQL::drop(db);}

    /* --------------------------------------------------------------------- */
    int replace_attribute(SqliteDB * db, Attribute * attr)
        {return TemplateSQL::replace_attribute(db,attr);}
    int insert_attribute(SqliteDB * db, Attribute * attr)
        {return TemplateSQL::insert_attribute(db,attr);}
    /* --------------------------------------------------------------------- */
    int id(){return TemplateSQL::id;};
};

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

class TemplateSQLTest : public CppUnit::TestFixture 
{
private:
    SqliteDB *db;
    TSQL     *t;

    string filename;

    string template_ok;
    string template_xml;

    static void log(
        const char *            module,
        const Log::MessageType  type,
        const ostringstream&    message,
        const char *            filename = 0,
        Log::MessageType        clevel   = Log::ERROR)
    {
        cerr << message.str() << endl;
    };

public:
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */

    TemplateSQLTest()
    {
        filename = "template_sql.db";

        template_ok = 
            "#This line is a comment\n"
            "  # Other comment\n"
            "MEMORY=345 # more comments behind an attribute\n"
            "   CPU     =         4\n"
            "#------------------------------\n"
            "#Comments in the middle\n"
            "#------------------------------\n"
            " empty_var = \n"
            "REQUIREMENTS = \"HOSTNAME = \\\"host*.com\\\"\"\n"
            " DISK= [file = path1, extra = \"disk attribute \"]\n"
            "DISK =[ FILE = \"path2\", EXTRA = str, TYPE=disk]\n"
            " graphics = [\n"
            "   VNC = \"127.0.0.1\",\n"
            "   PORT = 12\n"
            " ]\n";

        template_xml = 
            "<TEMPLATE><CPU>4</CPU><DISK><EXTRA>disk "
            "attribute </EXTRA><FILE>path1</FILE></DISK><DISK><EXTRA>str"
            "</EXTRA><FILE>path2</FILE>"
            "<TYPE>disk</TYPE></DISK><EMPTY_VAR></EMPTY_VAR><GRAPHICS>"
            "<PORT>12</PORT><VNC>127.0.0.1</VNC></GRAPHICS><MEMORY>345</MEMORY>"
            "<REQUIREMENTS>HOSTNAME = \"host*.com\"</REQUIREMENTS></TEMPLATE>";
    }

    ~TemplateSQLTest()
    {
        unlink("template_sql.db");
    }

    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */

    void setUp()
    {
        db = new SqliteDB (filename , TemplateSQLTest::log);
        t  = new TSQL("template");
    }

    void tearDown()
    {
        delete db;
        delete t;
    }

    /* ********************************************************************* */
    /* ********************************************************************* */

    void test_bootstrap()
    {
        const char * db_bs = "CREATE TABLE template"
                       " (id INTEGER, name TEXT, type INTEGER, value TEXT)";

        CPPUNIT_ASSERT( db->exec(db_bs) == 0);
    }

    /* --------------------------------------------------------------------- */
    
    void test_insert()
    {
        char * error = 0;
        int    rc;
        string tmp;

        rc = t->parse(template_ok,&error);
        
        if ( error != 0 )
        {
            cerr << error << endl;
            free(error);
        }

        CPPUNIT_ASSERT( rc == 0);

        CPPUNIT_ASSERT( t->insert(db) == 0 ); 
        CPPUNIT_ASSERT( t->id() == 0 ); 

        CPPUNIT_ASSERT( t->insert(db) == 0 ); 
        CPPUNIT_ASSERT( t->id() == 1 ); 

        CPPUNIT_ASSERT( t->insert(db) == 0 ); 
        CPPUNIT_ASSERT( t->id() == 2 ); 
    }

    /* --------------------------------------------------------------------- */

    void test_select()
    {
        TSQL t2("template",1);
        string t2_xml;

        CPPUNIT_ASSERT( t2.select(db) == 0 );

        t2.to_xml(t2_xml);

        CPPUNIT_ASSERT( t2_xml == template_xml );

    }

    /* ********************************************************************* */
    /* ********************************************************************* */

    static CppUnit::TestSuite * suite()
    {
        CppUnit::TestSuite *ts=new CppUnit::TestSuite("TemplateSQL Tests");

        ts->addTest(new CppUnit::TestCaller<TemplateSQLTest>(
                    "db() Test",
                    &TemplateSQLTest::test_bootstrap));

        ts->addTest(new CppUnit::TestCaller<TemplateSQLTest>(
                    "insert() Test",
                    &TemplateSQLTest::test_insert));

        ts->addTest(new CppUnit::TestCaller<TemplateSQLTest>(
                    "update() Test",
                    &TemplateSQLTest::test_select));

        return ts;
    }
};

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char ** argv)
{
    CppUnit::TextUi::TestRunner tr;
    
    tr.addTest(TemplateSQLTest::suite());
    tr.run();

    return 0;
}
