/*
 * Panl.c
 */
 
#include <Hs.h>
#include <PalmOS.h>
#include <Form.h>
#include <palmOneResources.h>

#include "AppResources.h"

//----------------------------------------------------
// Global variables
//----------------------------------------------------
// Different UI if called from app ("done" button and no selection list)
// static Boolean             CalledFromAppMode;

// What's required to list the panels
static UInt16              panelCount;
static MemHandle           panelIDsH = 0;

extern FormPtr 				pForm;

void PanelFormClose(FormPtr pForm);

//----------------------------------------------------
// PanelFormClose()
// Clean-up after form is closed
//----------------------------------------------------
void PanelFormClose(FormPtr pForm)
{
	// Free memory
	if (panelIDsH != 0)
	{
		if (panelCount > 0)
		{
			MemHandleUnlock(panelIDsH);
		}
		MemHandleFree(panelIDsH);
	}
	FrmEraseForm(pForm);
	FrmDeleteForm(pForm);
}

/*
 * Panl.c
 */
