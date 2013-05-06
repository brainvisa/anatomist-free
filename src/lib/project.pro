TEMPLATE	= lib
TARGET		= anatomist${BUILDMODEEXT}

#!include ../../config

DEFINES	+= ANATOMIST_EXPORTS

HEADERS = \
	browser/browsercontrol.h \
	browser/labeledit.h \
	browser/attributedchooser.h \
	browser/stringEdit.h \
	browser/attDescr.h \
	browser/qObjBrowserWid.h \
	browser/qwObjectBrowser.h \
	bucket/Bucket.h \
	color/colortraits.h \
	color/qwObjPalette.h \
	color/wObjPalette.h \
	color/objectPalette.h \
	color/paletteList.h \
	color/palette.h \
	color/Material.h \
	color/Light.h \
	color/wMaterial.h \
	color/wRendering.h \
	commands/cAddObject.h \
	commands/cApplyBuiltinReferential.h \
	commands/cAskTexExtrema.h \
        commands/cCamera.h \
	commands/cChangePalette.h \
	commands/cControlsParams.h \
	commands/cClosePipe.h \
	commands/cCloseWindow.h \
	commands/cCreateControlWindow.h \
	commands/cCreateWindow.h \
        commands/cDeleteAll.h \
	commands/cDeleteElement.h \
	commands/cDeleteObject.h \
	commands/cDuplicateObject.h \
	commands/cEventFilter.h \
	commands/cExit.h \
	commands/cExportTexture.h \
	commands/cExternalReference.h \
	commands/cExtractTexture.h \
	commands/cFusion2DParams.h \
	commands/cFusion3DParams.h \
        commands/cFusionInfo.h \
        commands/cFusionObjects.h \
        commands/cGenerateTexture.h \
	commands/cGetInfo.h \
	commands/cGraphDisplayProperties.h \
	commands/cGraphParams.h \
	commands/cGroupObjects.h \
	commands/cLinkedCursor.h \
	commands/cLinkWindows.h \
        commands/cLoadGraphSubObjects.h \
	commands/cLoadObject.h \
	commands/cNewId.h \
	commands/cNewPalette.h \
	commands/cObjectInfo.h \
	commands/cOutput.h \
	commands/cPopupPalette.h \
	commands/cReloadObject.h \
	commands/cRemoveObject.h \
	commands/cAssignReferential.h \
	commands/cLoadTransformation.h \
	commands/cSaveObject.h \
	commands/cSaveTransformation.h \
	commands/cSelect.h \
	commands/cSelectByHierarchy.h \
        commands/cServer.h \
        commands/cSetControl.h \
	commands/cSetMaterial.h \
        commands/cSetObjectPalette.h \
        commands/cShowObject.h \
	commands/cSliceParams.h \
	commands/cTexturingParams.h \
        commands/cWindowBlock.h \
	commands/cWindowConfig.h \
        config/anatomist_config.h \
	dialogs/colorDialog.h \
	dialogs/colorWidget.h \
	dialogs/qScopeLineEdit.h \
	hierarchy/hierarchy.h \
	selection/qwSelAttrib.h \
	selection/qSelMenu.h \
	selection/qSelectFactory.h \
	selection/qSelectWid.h \
	selection/wSelChooser.h \
	selection/selectFactory.h \
	selection/selector.h \
	selection/postSelector.h \
	misc/error.h \
	observer/Observer.h \
	observer/Observable.h \
	observer/observcreat.h \
	reference/Transformation.h \
	reference/Referential.h \
	reference/Geometry.h \
	reference/refobserver.h \
        reference/refpixmap.h \
	reference/transformobserver.h \
	reference/transfSet.h \
	reference/wReferential.h \
	reference/wChooseReferential.h \
	surface/cutmesh.h \
	surface/fusion2Dmesh.h \
	surface/fusiontexsurf.h \
	surface/glcomponent.h \
        surface/glcomponent_internals.h \
	surface/globject.h \
	surface/mtexture.h \
	surface/planarfusion3d.h \
	surface/surface.h \
	surface/surface_d.h \
	surface/tesselatedmesh.h \
	surface/texsurface.h \
        surface/textobject.h \
	surface/texture.h \
	surface/transformedobject.h \
	surface/triangulated.h \
	surface/Shader.h \
	graph/AGraphIterator.h \
	graph/attribAObject.h \
	graph/pythonAObject.h \
	graph/qgraphproperties.h \
	graph/GraphObject.h \
	graph/Graph.h \
	object/actions.h \
	object/clippedobject.h \
        object/loadevent.h \
	object/objectmenuCallbacks.h \
        object/objectmenu.h \
	object/objectparamselect.h \
	object/optionMatcher.h \
	object/oReader.h \
	object/Object.h \
        object/objectConverter.h \
	object/qtextureparams.h \
	object/selfsliceable.h \
	object/sliceable.h \
	object/texturepanel.h \
	object/texturewin.h \
	control/coloredpixmap.h \
	control/graphParams.h \
	control/qAbout.h \
	control/qWinTree.h \
	control/qObjTree.h \
#	control/toolTips.h \
	control/toolTips-qt.h \
	control/winconfigio.h \
	control/wControl.h \
	control/controlMenuHandler.h \
	control/abstractMenuHandler.h \
	control/qImageLabel.h \
	control/wPreferences.h \
	control/controlConfig.h \
	control/whatsNew.h \
	control/objectDrag.h \
	control/backPixmap_P.h \
	controler/action.h \
	controler/control_d.h \
	controler/actiondictionary.h \
	controler/controldictionary.h \
	controler/controlgroupdictionary.h \
	controler/icondictionary.h \
	controler/actionpool.h \
	controler/controlmanager.h \
	controler/control.h \
	controler/controlswitch.h \
	controler/view.h \
	control/windowdrag.h \
	window/colorstyle.h \
	window/controlledWindow.h \
	window/glcaps.h \
	window/glcontext.h \
	window/glwidget.h \
	window/glwidgetmanager.h \
	window/qwinblock.h \
	window/qwindow.h \
	window/qWinFactory.h \
	window/viewstate.h \
	window/winFactory.h \
	window/Window.h \
    window3D/annotedgraph.h \
    window3D/orientationAnnotation.h \
	window3D/cursor.h \
	window3D/glwidget3D.h \
	window3D/control3D.h \
	window3D/labeleditaction.h \
	window3D/trackball.h \
	window3D/trackcut.h \
	window3D/trackOblique.h \
	window3D/trackObliqueSlice.h \
        window3D/transformer.h \
	window3D/window3D.h \
	window3D/wFixedPointOfView.h \
	window3D/wLightModel.h \
	window3D/wTools3D.h \
	window3D/zoomDialog.h \
	mobject/globjectlist.h \
	mobject/globjectvector.h \
	mobject/glmobject.h \
	mobject/objectVector.h \
	mobject/MObject.h \
	mobject/Fusion2D.h \
	mobject/Fusion3D.h \
	mobject/ObjectList.h \
	mobject/wFusion2D.h \
	mobject/wFusion3D.h \
	volume/slice.h \
	volume/Volume.h \
        processor/context.h \
	processor/errormessage.h \
	processor/event.h \
	processor/event_doc.h \
	processor/pipeReader.h \
	processor/pipeReaderP.h \
	processor/unserializer.h \
        processor/server.h \
	processor/Command.h \
	processor/Processor.h \
	processor/Registry.h \
	processor/Reader.h \
	processor/Writer.h \
	processor/Serializer.h \
	fusion/defFusionMethods.h \
	fusion/fusionFactory.h \
	fusion/fusionChooser.h \
        application/anatomistinfo.h \
	application/Anatomist.h \
	application/globalConfig.h \
	application/localConfig.h \
	application/settings.h \
	application/module.h \
	application/fileDialog.h \
	application/syntax.h \
	application/listDir.h \
	application/anatomist_doc.h \
	stdmod/stdmod.h \
        sparsematrix/sparsematrix.h \
	primitive/primitive.h \
	constrainteditor/wConstraintEditor.h

SOURCES = \
	browser/browsercontrol.cc \
	browser/labeledit.cc \
	browser/attributedchooser.cc \
	browser/stringEdit.cc \
	browser/attDescr.cc \
	browser/qObjBrowserWid.cc \
	browser/qwObjectBrowser.cc \
	bucket/Bucket.cc \
	color/colortraits.cc \
	color/qwObjPalette.cc \
	color/wObjPalette.cc \
	color/objectPalette.cc \
	color/paletteList.cc \
	color/palette.cc \
	color/Material.cc \
	color/Light.cc \
	color/wMaterial.cc \
	color/wRendering.cc \
	commands/cAddObject.cc \
	commands/cAskTexExtrema.cc \
	commands/cApplyBuiltinReferential.cc \
	commands/cAssignReferential.cc \
        commands/cCamera.cc \
	commands/cChangePalette.cc \
	commands/cControlsParams.cc \
	commands/cClosePipe.cc \
	commands/cCloseWindow.cc \
	commands/cCreateControlWindow.cc \
	commands/cCreateWindow.cc \
        commands/cDeleteAll.cc \
	commands/cDeleteElement.cc \
	commands/cDeleteObject.cc \
	commands/cDuplicateObject.cc \
	commands/cEventFilter.cc \
	commands/cExit.cc \
	commands/cExportTexture.cc \
	commands/cExternalReference.cc \
	commands/cExtractTexture.cc \
	commands/cFusion2DParams.cc \
	commands/cFusion3DParams.cc \
        commands/cFusionInfo.cc \
        commands/cFusionObjects.cc \
	commands/cGenerateTexture.cc \
	commands/cGetInfo.cc \
	commands/cGraphDisplayProperties.cc \
	commands/cGraphParams.cc \
	commands/cGroupObjects.cc \
	commands/cLinkedCursor.cc \
	commands/cLinkWindows.cc \
        commands/cLoadGraphSubObjects.cc \
	commands/cLoadObject.cc \
	commands/cLoadTransformation.cc \
	commands/cNewId.cc \
	commands/cNewPalette.cc \
	commands/cObjectInfo.cc \
	commands/cOutput.cc \
	commands/cPopupPalette.cc \
	commands/cReloadObject.cc \
	commands/cRemoveObject.cc \
	commands/cSaveObject.cc \
	commands/cSaveTransformation.cc \
	commands/cSelect.cc \
	commands/cSelectByHierarchy.cc \
        commands/cServer.cc \
        commands/cSetControl.cc \
	commands/cSetMaterial.cc \
        commands/cSetObjectPalette.cc \
        commands/cShowObject.cc \
	commands/cSliceParams.cc \
	commands/cTexturingParams.cc \
        commands/cWindowBlock.cc \
	commands/cWindowConfig.cc \
	dialogs/colorDialog.cc \
	dialogs/colorWidget.cc \
	dialogs/qScopeLineEdit.cc \
	hierarchy/hierarchy.cc \
	selection/qwSelAttrib.cc \
	selection/qSelMenu.cc \
	selection/qSelectFactory.cc \
	selection/qSelectWid.cc \
	selection/wSelChooser.cc \
	selection/selectFactory.cc \
	selection/selector.cc \
	selection/postSelector.cc \
	misc/error.cc \
	observer/Observable.cc \
	observer/observcreat.cc \
	observer/Observer.cc \
	reference/Transformation.cc \
	reference/Referential.cc \
	reference/Geometry.cc \
	reference/refobserver.cc \
        reference/refpixmap.cc \
        reference/transformobserver.cc \
	reference/transfSet.cc \
	reference/wReferential.cc \
	reference/wChooseReferential.cc \
	surface/cutmesh.cc \
	surface/fusion2Dmesh.cc \
	surface/fusiontexsurf.cc \
	surface/glcomponent.cc \
	surface/globject.cc \
	surface/mtexture.cc \
	surface/planarfusion3d.cc \
	surface/tesselatedmesh.cc \
	surface/texsurface.cc \
        surface/textobject.cc \
	surface/texture.cc \
        surface/transformedobject.cc \
	surface/triangulated.cc \
	surface/Shader.cc \
	graph/AGraphIterator.cc \
	graph/attribAObject.cc \
	graph/pythonAObject.cc \
	graph/qgraphproperties.cc \
	graph/GraphObject.cc \
	graph/Graph.cc \
	object/actions.cc \
	object/clippedobject.cc \
        object/loadevent.cc \
        object/objectmenu.cc \
	object/objectparamselect.cc \
	object/optionMatcher.cc \
	object/oReader.cc \
	object/Object.cc \
        object/objectConverter.cc \
	object/qtextureparams.cc \
	object/selfsliceable.cc \
	object/sliceable.cc \
	object/texturepanel.cc \
	object/texturewin.cc \
	object/objectmenuCallbacks.cc \
	control/coloredpixmap.cc \
	control/graphParams.cc \
	control/qAbout.cc \
	control/qAboutMessages.cc \
	control/qWinTree.cc \
	control/qObjTree.cc \
#	control/toolTips.cc \
	control/toolTips-qt.cc \
	control/winconfigio.cc \
	control/wControl.cc \
	control/controlMenuHandler.cc \
	control/abstractMenuHandler.cc \
	control/qImageLabel.cc \
	control/wPreferences.cc \
	control/controlConfig.cc \
	control/whatsNew.cc \
	control/objectDrag.cc \
	control/backPixmap_P.cc \
	control/windowdrag.cc \
        controler/action.cc \
	controler/actiondictionary.cc \
	controler/controldictionary.cc \
	controler/controlgroupdictionary.cc \
	controler/icondictionary.cc \
	controler/actionpool.cc \
	controler/controlmanager.cc \
	controler/control.cc \
	controler/controlswitch.cc \
	controler/view.cc \
	window/colorstyle.cc \
	window/controlledWindow.cc \
	window/glcaps.cc \
	window/glcontext.cc \
	window/glwidget.cc \
	window/glwidgetmanager.cc \
	window/qwinblock.cc \
	window/qwindow.cc \
	window/qWinFactory.cc \
	window/viewstate.cc \
	window/winFactory.cc \
	window/Window.cc \
    window3D/annotedgraph.cc \
    window3D/orientationAnnotation.cc \
	window3D/cursor.cc \
	window3D/glwidget3D.cc \
	window3D/control3D.cc \
	window3D/labeleditaction.cc \
	window3D/trackball.cc \
	window3D/trackcut.cc \
	window3D/trackOblique.cc \
	window3D/trackObliqueSlice.cc \
	window3D/transformer.cc \
	window3D/window3D.cc \
	window3D/wFixedPointOfView.cc \
	window3D/wLightModel.cc \
	window3D/wTools3D.cc \
	window3D/zoomDialog.cc \
	mobject/globjectlist.cc \
	mobject/globjectvector.cc \
	mobject/glmobject.cc \
	mobject/objectVector.cc \
	mobject/MObject.cc \
	mobject/Fusion2D.cc \
	mobject/Fusion3D.cc \
	mobject/ObjectList.cc \
	mobject/wFusion2D.cc \
	mobject/wFusion3D.cc \
	volume/slice.cc \
	volume/Volume.cc \
        processor/context.cc \
	processor/errormessage.cc \
	processor/event.cc \
	processor/pipeReader.cc \
	processor/unserializer.cc \
        processor/server.cc \
	processor/Command.cc \
	processor/Processor.cc \
	processor/Registry.cc \
	processor/Reader.cc \
	processor/Writer.cc \
	processor/Serializer.cc \
	fusion/defFusionMethods.cc \
	fusion/fusionFactory.cc \
	fusion/fusionChooser.cc \
        application/anatomistinfo.cc \
        application/statics.cc	\
	application/Anatomist.cc \
	application/globalConfig.cc \
	application/localConfig.cc \
	application/settings.cc \
	application/module.cc \
	application/fileDialog.cc \
	application/syntax.cc \
	application/listDir.cc \
	sparsematrix/sparsematrix.cc \
	stdmod/stdmod.cc \
	primitive/primitive.cc \
	constrainteditor/wConstraintEditor.cc

#release-darwin:LIBS	= $(DARWINBUG_LIBS)

