#include "stdafx.h"

#include "ReaperUtils.h"

#include "wdltypes.h"

//#include "swell/swell.h"
//#include "swell/swell-functions.h"
#include "swell/swell-dlggen.h"



// arbitrary unique identifiers
enum DialogID { IDD_NETCONF_DIALOG = 100 };
enum ControlID { IDC_LABEL = 200, IDC_PROXY, IDC_LABEL2, IDC_STALETHRSH, IDC_VERIFYPEER };

SWELL_DialogResourceIndex SWELL_curmodule_dialogresource_head;

// Register a new dialog ID
// This SWELL_DialogRegHelper object must live forever
// (or update SWELL_curmodule_dialogresource_head when destructing)
static SWELL_DialogRegHelper Register_IDD_NETCONF_DIALOG{
  &SWELL_curmodule_dialogresource_head, [](HWND view, int wflags)
  {
	// Construct a new dialog (HWND view)
	SWELL_MakeSetCurParms(1.8, 1.8, 0, 0, view, false, !(wflags & 8));

	SWELL_MakeLabel(-1, "Proxy:", IDC_LABEL, 5, 8, 20, 10, 0);
	SWELL_MakeEditField(IDC_PROXY, 30, 5, 185, 14, ES_AUTOHSCROLL);
	SWELL_MakeLabel(-1, "Example: host:port, [ipv6]:port or scheme://host:port",
	  IDC_LABEL2, 30, 22, 190, 10, 0);
	SWELL_MakeCheckBox("&Refresh index cache when older than one week",
	  IDC_STALETHRSH, 5, 33, 220, 14, BS_AUTOCHECKBOX | WS_TABSTOP);
	SWELL_MakeCheckBox("&Verify the authenticity of SSL/TLS certificates (advanced)",
	  IDC_VERIFYPEER, 5, 45, 220, 14, BS_AUTOCHECKBOX | WS_TABSTOP);
	SWELL_MakeButton(1, "&OK", IDOK, 132, 61, 40, 14, 0);
	SWELL_MakeButton(0, "&Cancel", IDCANCEL, 175, 61, 40, 14, 0);
  },
  IDD_NETCONF_DIALOG, 4 | 8, "Network settings", 220, 80, 1.8
};

// IDD_NETCONF_DIALOG can now be used