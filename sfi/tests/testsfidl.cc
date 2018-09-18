// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#undef G_LOG_DOMAIN
#define  G_LOG_DOMAIN __FILE__
#include "../sfidl-generator.hh"
#include "../sfidl-factory.hh"
#include <stdio.h>
#include <assert.h>

// #include <sfi/testing.hh>
#define TESTING(m)      fprintf (stdout, "  TESTING  %s\n", m)
#define TASSERT(a)      assert (a)
#define ASSERT_EQ(a,b)  assert (a == b)

using namespace Sfidl;
using namespace std;

class TestCG : public CodeGenerator
{
  string one, two, done;
public:
  TestCG(const Parser& parser) : CodeGenerator (parser)
  {
  }
  OptionVector getOptions()
  {
    OptionVector opts;

    opts.push_back (make_pair ("--one", true));
    opts.push_back (make_pair ("--two", true));
    opts.push_back (make_pair ("--done", false));

    return opts;
  }
  void setOption (const string& option, const string& value)
  {
    if (option == "--one") one = value;
    if (option == "--two") two = value;
    if (option == "--done") done = value;
  }
  bool run()
  {
    TESTING ("Testing Option parser");
    ASSERT_EQ (one, "1");
    ASSERT_EQ (two, "2");
    ASSERT_EQ (done, "1");
    TESTING ("Testing CodeGenerator::rename()");
    vector<string> procedures;
    vector<string> empty;
    vector<string> type;
    procedures.push_back("Procedures");
    type.push_back("Type");
    TASSERT (procedures.size() == 1);
    TASSERT (type.size() == 1);
    TASSERT (empty.size() == 0);
    ASSERT_EQ (rename (ABSOLUTE, "A::B::processMessagesSlowly", Capitalized, "::",
		       procedures, lower, "_"),
               "::A::B::Procedures::process_messages_slowly");
    ASSERT_EQ (rename (NONE, "A::B::processMessagesSlowly", Capitalized, "::",
		       procedures, lower, "_"),
	       "Procedures::process_messages_slowly");
    ASSERT_EQ (rename (ABSOLUTE, "Bse::SNet", Capitalized, "",
		       empty, Capitalized, ""),
	       "BseSNet");
    ASSERT_EQ (rename (ABSOLUTE, "Bse::AlphaSynth", UPPER, "_",
		       type, UPPER, "_"),
	       "BSE_TYPE_ALPHA_SYNTH");
    ASSERT_EQ (rename (ABSOLUTE, "Bse::FFTSize", Capitalized, "",
                       empty, Capitalized, ""),
              "BseFFTSize");
    ASSERT_EQ (rename (ABSOLUTE, "Bse::FFTSize", lower, "_",
                       empty, lower, "_"),
              "bse_fft_size");
    ASSERT_EQ (rename (ABSOLUTE, "XWindows::WMHints", Capitalized, "",
                       empty, Capitalized, ""),
              "XWindowsWMHints");
    ASSERT_EQ (rename (ABSOLUTE, "XWindows::WMHints", UPPER, "_",
                       empty, UPPER, "_"),
              "XWINDOWS_WM_HINTS");
    ASSERT_EQ (rename (ABSOLUTE, "Bse::MidiSignal::PROGRAM", UPPER, "_",
                       empty, UPPER, "_"),
              "BSE_MIDI_SIGNAL_PROGRAM");
    return true;
  }
};
class TestCGFactory : public Factory {
public:
  string option() const	      { return "--test"; }
  string description() const  { return "test code generator"; }
  CodeGenerator *create (const Parser& parser) const
  {
    return new TestCG (parser);
  }
} static_factory;

int
main (int   argc,
      char *argv[])
{
  Options options;
  Parser parser;
  int fake_argc = 6;
  char **fake_argv = g_new0 (gchar*, fake_argc);
  fake_argv[0] = (char*) "testsfidl";
  fake_argv[1] = (char*) "--test";
  fake_argv[2] = (char*) "--one";
  fake_argv[3] = (char*) "1";
  fake_argv[4] = (char*) "--two=2";
  fake_argv[5] = (char*) "--done";
  options.parse (&fake_argc, &fake_argv, parser);

  TESTING ("Sfidl factory");
  TASSERT (options.codeGenerator != 0);

  if (options.codeGenerator->run())
    {
      delete options.codeGenerator;
      return 0;
    }
  else
    {
      delete options.codeGenerator;
      return 1;
    }
}

#include "../sfidl.cc"

/* vim:set ts=8 sts=2 sw=2: */
