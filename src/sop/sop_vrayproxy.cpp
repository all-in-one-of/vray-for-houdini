//
// Copyright (c) 2015-2017, Chaos Software Ltd
//
// V-Ray For Houdini
//
// ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
//
// Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
//

#include "sop_vrayproxy.h"

#include "vfh_vrayproxyutils.h"
#include "vfh_prm_templates.h"

using namespace VRayForHoudini;

/// Callback to clear cache for this node ("Reload Geometry" button in the GUI).
/// @param data Pointer to the node it was called on.
/// @param index The index of the menu entry.
/// @param t Current evaluation time.
/// @param tplate Pointer to the PRM_Template of the parameter it was triggered for.
/// @return It should return 1 if you want the dialog to refresh
/// (ie if you changed any values) and 0 otherwise.
static int cbClearCache(void *data, int index, fpreal t, const PRM_Template *tplate)
{
	OP_Node *node = reinterpret_cast<OP_Node*>(data);

	UT_String filepath;
	node->evalString(filepath, "file", 0, t);

	if (filepath.isstring()) {
		clearVRayProxyCache(filepath);
	}

	return 0;
}

PRM_Template* SOP::VRayProxy::getPrmTemplate()
{
	static PRM_Template* myPrmList = nullptr;
	if (myPrmList) {
		return myPrmList;
	}

	myPrmList = Parm::getPrmTemplate("GeomMeshFile");

	PRM_Template* prmIt = myPrmList;
	while (prmIt && prmIt->getType() != PRM_LIST_TERMINATOR) {
		if (vutils_strcmp(prmIt->getToken(), "reload") == 0) {
			prmIt->setCallback(cbClearCache);
			break;
		}
		prmIt++;
	}

	return myPrmList;
}

SOP::VRayProxy::VRayProxy(OP_Network *parent, const char *name, OP_Operator *entry)
	: NodePackedBase("VRayProxyRef", parent, name, entry)
{
	// This indicates that this SOP manually manages its data IDs,
	// so that Houdini can identify what attributes may have changed,
	// e.g. to reduce work for the viewport, or other SOPs that
	// check whether data IDs have changed.
	// By default, (i.e. if this msg weren't here), all data IDs
	// would be bumped after the SOP cook, to indicate that
	// everything might have changed.
	// If some data IDs don't get bumped properly, the viewport
	// may not update, or SOPs that check data IDs
	// may not cook correctly, so be *very* careful!
	// XXX: Is this still required?
	// mySopFlags.setManagesDataIDs(true);
}
void SOP::VRayProxy::setPluginType()
{
	pluginType = VRayPluginType::GEOMETRY;
	pluginID   = "GeomMeshFile";
}

void SOP::VRayProxy::setTimeDependent()
{
	const VUtils::MeshFileAnimType::Enum animType =
		static_cast<VUtils::MeshFileAnimType::Enum>(evalInt("anim_type", 0, 0.0));
	flags().setTimeDep(animType != VUtils::MeshFileAnimType::Still);
}

void SOP::VRayProxy::updatePrimitive(const OP_Context &context)
{
	const fpreal t = context.getTime();

	// Set the options on the primitive
	OP_Options primOptions;
	for (int i = 0; i < getParmList()->getEntries(); ++i) {
		const PRM_Parm &prm = getParm(i);
		primOptions.setOptionFromTemplate(this, prm, *prm.getTemplatePtr(), t);
	}

	// XXX: What was this doing? Check if setOptionFromTemplate() sets this.
	UT_String objectPath;
	evalString(objectPath, "object_path", 0, 0.0);
	primOptions.setOptionS("object_path", objectPath);

	primOptions.setOptionI("preview_type", evalInt("preview_type", 0, 0.0));
	primOptions.setOptionF("current_frame", flags().getTimeDep() ? context.getFloatFrame() : 0.0f);
	
	updatePrimitiveFromOptions(primOptions);
}
