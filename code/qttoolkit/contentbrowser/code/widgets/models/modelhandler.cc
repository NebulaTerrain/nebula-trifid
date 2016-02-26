//------------------------------------------------------------------------------
//  modelitemhandler.cc
//  (C) 2012-2014 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "modelhandler.h"
#include "previewer/previewstate.h"
#include "contentbrowserapp.h"
#include "n3util/n3xmlextractor.h"
#include "io/ioserver.h"
#include "ui_modelnodeinfowidget.h"
#include "graphics/graphicsprotocol.h"
#include "modelnodeframe.h"
#include "particlenodeframe.h"
#include "n3util/n3xmlexporter.h"
#include "graphics/graphicsinterface.h"
#include "characternodeframe.h"
#include "modelutil/modelbuilder.h"
#include "modelnodehandler.h"
#include "qtaddons/remoteinterface/qtremoteclient.h"
#include "messaging/messagecallbackhandler.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QBitmap>
#include "physicsnodeframe.h"

using namespace ContentBrowser;
using namespace ToolkitUtil;
using namespace Util;
using namespace IO;
using namespace Graphics;
using namespace Actions;
using namespace Particles;

namespace Widgets
{
__ImplementClass(Widgets::ModelHandler, 'MOIH', Widgets::BaseHandler);

//------------------------------------------------------------------------------
/**
*/
ModelHandler::ModelHandler() : 
	attributes(0),
	constants(0),
	physics(0),
	characterFrame(0)
{
	this->action = ModelModifyAction::Create();
	this->action->SetModelHandler(this);
}

//------------------------------------------------------------------------------
/**
*/
ModelHandler::~ModelHandler()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::Cleanup()
{
    // cleanup action
    this->action->Cleanup();
    this->action = 0;

    // remove intermediate directory
    IoServer::Instance()->DeleteDirectory("int:");
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelHandler::Preview()
{
	// get preview state
	Ptr<PreviewState> previewState = ContentBrowserApp::Instance()->GetPreviewState();

	// create original resource uri
	String resource;
	resource.Format("mdl:%s/%s.n3", this->category.AsCharPtr(), this->file.AsCharPtr());

	// create original physics resource uri
	String phResource;
	phResource.Format("phys:%s/%s.np3", this->category.AsCharPtr(), this->file.AsCharPtr());

	// preview the model
	if (!previewState->SetModel(resource))
	{
		// just abort the rest
		return false;
	}

	if (!previewState->SetPhysics(phResource))
	{ 
		QMessageBox::critical(NULL, "Failed to open physics", "Failed to open physics resource");
	}
	return true;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::Setup()
{
	n_assert(this->ui);

	// enable the UI again
	this->ui->frame->setDisabled(false);
	this->ui->modelName->setText((this->category + "/" + this->file).AsCharPtr());

	this->ui->saveButton->setEnabled(true);
	this->ui->saveAsButton->setEnabled(true);
	this->ui->reconfigureButton->setEnabled(true);
	this->ui->addParticleNode->setEnabled(true);

	// call base class
	BaseHandler::Setup();	

	// set category and model of action
	this->action->SetCategory(this->category);
	this->action->SetModel(this->file);

	// get model data, force a reload
	this->attributes = ModelDatabase::Instance()->LookupAttributes(this->category + "/" + this->file, true);
	this->constants = ModelDatabase::Instance()->LookupConstants(this->category + "/" + this->file, true);
	this->physics = ModelDatabase::Instance()->LookupPhysics(this->category + "/" + this->file, true);

	// save original version
	this->OnNewVersion();

	// open files
	String attrPath, constPath, physPath;
	attrPath.Format("src:assets/%s/%s.attributes", this->category.AsCharPtr(), this->file.AsCharPtr());
	constPath.Format("src:assets/%s/%s.constants", this->category.AsCharPtr(), this->file.AsCharPtr());
	physPath.Format("src:assets/%s/%s.physics", this->category.AsCharPtr(), this->file.AsCharPtr());

	// update thumbnail
	this->UpdateModelThumbnail();

	// open file
	if (!IoServer::Instance()->FileExists(attrPath))
    {
		QMessageBox box;
		QString message;
		message.sprintf("The model '%s' either has inaccessible attributes or they are missing!", attrPath.AsCharPtr());
		box.setText(message);
		box.setIcon(QMessageBox::Warning);
		box.setStandardButtons(QMessageBox::Close);
		box.setDefaultButton(QMessageBox::Close);
		box.exec();

		this->ui->saveButton->setEnabled(false);
		this->ui->saveAsButton->setEnabled(false);
		this->ui->reconfigureButton->setEnabled(false);
		this->ui->addParticleNode->setEnabled(false);

		// discard and return
		this->DiscardNoCancel();
		return;
    }

	// reset saved state
	this->ui->saveButton->setStyleSheet("background-color: rgb(4, 200, 0); color: white");
	this->SetupTabs();

	// connect reload buttons to actions
	connect(this->ui->saveButton, SIGNAL(clicked()), this, SLOT(OnSave()));
	connect(this->ui->saveAsButton, SIGNAL(clicked()), this, SLOT(OnSaveAs()));
	connect(this->ui->addParticleNode, SIGNAL(clicked()), this, SLOT(OnAddParticleNode()));
	connect(this->ui->reconfigureButton, SIGNAL(clicked()), this, SLOT(OnReconfigure()));
}

//------------------------------------------------------------------------------
/**
*/
bool 
ModelHandler::Discard()
{
	// before switching models, check if our action is at the final version
	if (!this->action->IsFinal())
	{
		// if we don't, we should prompt the user to save
		QMessageBox msgBox;
		msgBox.setText("The model has been modified.");
		msgBox.setInformativeText("Before you close this model, do you wish to save first?");
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Save);

		// execute
		int ret = msgBox.exec();

		if (ret == QMessageBox::Save)
		{
			this->OnSave();
		}
		else if (ret == QMessageBox::Cancel)
		{
			// abort action
			return false;
		}
	}

	// set new preview
	this->category = "system";
	this->file = "placeholder";
	this->Preview();

	// clear frames
	IndexT i;
	for (i = 0; i < this->nodeFrames.Size(); i++)
	{
		this->nodeFrames[i]->Discard();
	}
	for (i = 0; i < particleFrames.Size(); i++)
	{
		this->particleFrames[i]->Discard();
	}
	this->nodeFrames.Clear();
	this->particleFrames.Clear();

	if (this->characterFrame)
	{
		this->characterFrame->Discard();
		this->characterFrame = 0;
	}

    // clear attributes
    this->attributes->Clear();
    this->constants->Clear();
    this->physics->Clear();

	// clear model stuff
	this->attributes = 0;
	this->constants = 0;
	this->physics = 0;

	// discard current action
	this->action->Discard();

	// clear actions from application
	ContentBrowserApp::Instance()->ClearActions();

	// disconnect stuff
	disconnect(this->ui->saveButton, SIGNAL(clicked()), this, SLOT(OnSave()));
	disconnect(this->ui->saveAsButton, SIGNAL(clicked()), this, SLOT(OnSaveAs()));
	disconnect(this->ui->addParticleNode, SIGNAL(clicked()), this, SLOT(OnAddParticleNode()));

	return BaseHandler::Discard();
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::DiscardNoCancel()
{
	// before switching models, check if our action is at the final version
	if (!this->action->IsFinal())
	{
		// if we don't, we should prompt the user to save
		QMessageBox msgBox;
		msgBox.setText("The model has been reimported and must be updated.");
		msgBox.setInformativeText("Do you want to save your changes before updating?");
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
		msgBox.setDefaultButton(QMessageBox::Save);

		// execute
		int ret = msgBox.exec();

		if (ret == QMessageBox::Save)
		{
			this->OnSave();
		}
	}

	// clear frames
	IndexT i;
	for (i = 0; i < this->nodeFrames.Size(); i++)
	{
		this->nodeFrames[i]->Discard();
	}
	for (i = 0; i < particleFrames.Size(); i++)
	{
		this->particleFrames[i]->Discard();
	}
	this->nodeFrames.Clear();
	this->particleFrames.Clear();

	if (this->characterFrame)
	{
		this->characterFrame->Discard();
		this->characterFrame = 0;
	}

    // clear attributes
    this->attributes->Clear();
    this->constants->Clear();
    this->physics->Clear();

	// clear model stuff
	this->attributes = 0;
	this->constants = 0;
	this->physics = 0;

	// discard current action
	this->action->Discard();

	// clear actions
	ContentBrowserApp::Instance()->ClearActions();

	// set setup bool to false
	this->isSetup = false;

	// disconnect stuff
	disconnect(this->ui->saveButton, SIGNAL(clicked()), this, SLOT(OnSave()));
	disconnect(this->ui->saveAsButton, SIGNAL(clicked()), this, SLOT(OnSaveAs()));
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::Mute()
{
	this->ui->frame->setDisabled(true);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::Refresh()
{
	// clear frames
	IndexT i;
	for (i = 0; i < this->nodeFrames.Size(); i++)
	{
		this->nodeFrames[i]->Discard();
		delete this->nodeFrames[i];
	}
	for (i = 0; i < this->particleFrames.Size(); i++)
	{
		this->particleFrames[i]->Discard();
		delete this->particleFrames[i];
	}
	for (i = 0; i < this->physicsFrames.Size(); i++)
	{
		this->physicsFrames[i]->Discard();
		delete this->physicsFrames[i];
	}
	this->nodeFrames.Clear();
	this->particleFrames.Clear();
	this->physicsFrames.Clear();

	if (this->characterFrame)
	{
		this->characterFrame->Discard();
		delete this->characterFrame;
		this->characterFrame = 0;
	}

	this->SetupTabs();
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::OnModelModified(bool structureChange)
{
	// make sure to create a new version
	this->OnNewVersion();

	// mark save button
	this->ui->saveButton->setStyleSheet("background-color: rgb(200, 4, 0); color: white");

	// finally push action to stack
	ContentBrowserApp::Instance()->PushAction(this->action.upcast<BaseAction>());
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::OnNewVersion()
{
	// create memory stream to which modifier will write
	Ptr<MemoryStream> stream = MemoryStream::Create();
	this->attributes->Save(stream.upcast<Stream>());
	stream->SetAccessMode(Stream::ReadAccess);
	stream->Open();
	void* data = stream->Map();
	SizeT length = stream->GetSize();
	String attrVersion;
	attrVersion.Set((const char*)data, length);
	stream->Unmap();
	stream->Close();

	stream = MemoryStream::Create();
	this->constants->Save(stream.upcast<Stream>());
	stream->SetAccessMode(Stream::ReadAccess);
	stream->Open();
	data = stream->Map();
	length = stream->GetSize();
	String constsVersion;
	constsVersion.Set((const char*)data, length);
	stream->Unmap();
	stream->Close();

	stream = MemoryStream::Create();
	this->physics->Save(stream.upcast<Stream>());
	stream->SetAccessMode(Stream::ReadAccess);
	stream->Open();
	data = stream->Map();
	length = stream->GetSize();
	String physVersion;
	physVersion.Set((const char*)data, length);
	stream->Unmap();
	stream->Close();

	// add constants
	this->action->AddVersion(attrVersion, constsVersion, physVersion);

	// apply action
	this->action->DoAndMakeCurrent();
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::OnFrame()
{
	if (this->characterFrame) this->characterFrame->GetHandler()->OnFrame();
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::OnAddParticleNode()
{
	// get model data, force a reload
	Ptr<ModelAttributes> attrs = ModelDatabase::Instance()->LookupAttributes(this->category + "/" + this->file);
	Ptr<ModelConstants> consts = ModelDatabase::Instance()->LookupConstants(this->category + "/" + this->file);
	Ptr<ModelPhysics> phys = ModelDatabase::Instance()->LookupPhysics(this->category + "/" + this->file);

	// we should now have an empty particle effect, so now we need a node
	String name;
	IndexT number = consts->GetParticleNodes().Size();
	name.Format("node_%d", number);
	while (consts->HasParticleNode(name)) name.Format("node_%d", number++);

	ModelConstants::ParticleNode node;	
	node.name = name;
	node.type = "particle";
	node.path = "root/" + name;
	node.primitiveGroupIndex = 0;

	// now add particle node to constants
	consts->AddParticleNode(name, node);

	// create state
	State state;
	state.material = "sur:system/particle_unlit";
	state.textures.Append(Texture("DiffuseMap", "tex:system/nebulalogo"));
	state.textures.Append(Texture("NormalMap", "tex:system/nobump"));

	// set state in attributes for this node
	attrs->SetState(node.path, state);

	// create set of emitter attributes, setup defaults
	EmitterAttrs emitterAttrs;
	emitterAttrs.SetBool(EmitterAttrs::Looping, true);
	emitterAttrs.SetFloat(EmitterAttrs::EmissionDuration, 25.0f);
	emitterAttrs.SetFloat(EmitterAttrs::Gravity, -9.82f);
	emitterAttrs.SetFloat(EmitterAttrs::ActivityDistance, 100.0f);
	emitterAttrs.SetFloat(EmitterAttrs::StartRotationMin, 20.0f);
	emitterAttrs.SetFloat(EmitterAttrs::StartRotationMax, 60.0f);
	emitterAttrs.SetInt(EmitterAttrs::AnimPhases, 1);
	EnvelopeCurve lifeTime;
	lifeTime.Setup(1, 1, 1, 1, 0.33f, 0.66f, 0, 0, EnvelopeCurve::Sine);
	EnvelopeCurve emissionFrequency;
	emissionFrequency.Setup(25, 25, 25, 25, 0.33f, 0.66f, 1, 1, EnvelopeCurve::Sine);
	emissionFrequency.SetLimits(0, 25);
	EnvelopeCurve alpha;
	alpha.Setup(0, 0.5f, 0.5f, 0, 0.33f, 0.66f, 1, 0, EnvelopeCurve::Sine);
	EnvelopeCurve size;
	size.Setup(0, 0.5f, 0.5f, 0.0f, 0.33f, 0.66f, 1, 1, EnvelopeCurve::Sine);
	emitterAttrs.SetEnvelope(EmitterAttrs::LifeTime, lifeTime);
	emitterAttrs.SetEnvelope(EmitterAttrs::VelocityFactor, lifeTime);
	emitterAttrs.SetEnvelope(EmitterAttrs::EmissionFrequency, emissionFrequency);
	emitterAttrs.SetEnvelope(EmitterAttrs::Alpha, alpha);
	emitterAttrs.SetEnvelope(EmitterAttrs::Size, size);

	// set attributes
	attrs->SetEmitterAttrs(node.path, emitterAttrs);
	attrs->SetEmitterMesh(node.path, "");

	// save new changes and make a new model
	this->OnModelModified(true);

	// refresh the model handler
	this->MakeModel();
	this->Refresh();
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::RemoveParticleNode(const Util::String path, const Util::String node)
{
	Ptr<ModelAttributes> attrs = ModelDatabase::Instance()->LookupAttributes(this->category + "/" + this->file);
	Ptr<ModelConstants> consts = ModelDatabase::Instance()->LookupConstants(this->category + "/" + this->file);
	Ptr<ModelPhysics> phys = ModelDatabase::Instance()->LookupPhysics(this->category + "/" + this->file);

	// simply delete the node
	attrs->DeleteState(path);
	attrs->DeleteEmitterAttrs(path);
	attrs->DeleteEmitterMesh(path);
	consts->DeleteParticleNode(node);

	// apply modifications
	this->OnModelModified(true);

	// refresh the model handler
	this->MakeModel();
	this->Refresh();
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::OnSave()
{
	// firstly, tag current version to be final in the action
	this->action->TagAsFinal();

	// create resource string
	String resource;
	resource.Format("src:assets/%s/%s.attributes", this->category.AsCharPtr(), this->file.AsCharPtr());
	Ptr<Stream> stream = IoServer::Instance()->CreateStream(resource);
	stream->SetAccessMode(Stream::WriteAccess);
	stream->Open();
	String lastVersion = this->action->GetLastAttrVersion();
	stream->Write((void*)lastVersion.AsCharPtr(), lastVersion.Length());
	stream->Close();
	
	resource.Format("src:assets/%s/%s.constants", this->category.AsCharPtr(), this->file.AsCharPtr());
	stream = IoServer::Instance()->CreateStream(resource);
	stream->SetAccessMode(Stream::WriteAccess);
	stream->Open();
	lastVersion = this->action->GetLastConstVersion();
	stream->Write((void*)lastVersion.AsCharPtr(), lastVersion.Length());
	stream->Close();

	resource.Format("src:assets/%s/%s.physics", this->category.AsCharPtr(), this->file.AsCharPtr());
	stream = IoServer::Instance()->CreateStream(resource);
	stream->SetAccessMode(Stream::WriteAccess);
	stream->Open();
	lastVersion = this->action->GetLastPhysVersion();
	stream->Write((void*)lastVersion.AsCharPtr(), lastVersion.Length());
	stream->Close();

	// create model builder
	Ptr<ModelBuilder> modelBuilder = ModelBuilder::Create();

	// set data for model builder
	modelBuilder->SetAttributes(this->attributes);
	modelBuilder->SetConstants(this->constants);
	modelBuilder->SetPhysics(this->physics);

	// create original resource uri
	resource.Format("mdl:%s/%s.n3", this->category.AsCharPtr(), this->file.AsCharPtr());

	// save model to temporary
	modelBuilder->SaveN3(resource, Platform::Win32);

	// send modification message to remote end
	Ptr<ReloadResourceIfExists> msg = ReloadResourceIfExists::Create();
	msg->SetResourceName(resource);
	QtRemoteInterfaceAddon::QtRemoteClient::GetClient("editor")->Send(msg.upcast<Messaging::Message>());

	// save physics as well
	resource.Format("physics:%s/%s.np3", this->category.AsCharPtr(), this->file.AsCharPtr());
	modelBuilder->SaveN3Physics(resource, Platform::Win32);

	// also send physics update
	msg = ReloadResourceIfExists::Create();
	msg->SetResourceName(resource);
	QtRemoteInterfaceAddon::QtRemoteClient::GetClient("editor")->Send(msg.upcast<Messaging::Message>());

	// generate thumbnail
	String thumbnail = String::Sprintf("src:assets/%s/%s_n3.thumb", this->category.AsCharPtr(), this->file.AsCharPtr());

	// get preview state
	Ptr<PreviewState> previewState = ContentBrowserApp::Instance()->GetPreviewState();
	previewState->SaveThumbnail(thumbnail, true);

	// mark save button
	this->ui->saveButton->setStyleSheet("background-color: rgb(4, 200, 0); color: white");

	// update thumbnail
	this->UpdateModelThumbnail();
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelHandler::OnSaveAs()
{
	// perform a modify which will create a new action
	this->OnModelModified();

	// firstly, tag current version to be final in the action
	this->action->TagAsFinal();

	QFileDialog fileDialog(NULL, "Save Model As", IO::URI("mdl:").GetHostAndLocalPath().AsCharPtr(), "*.n3");
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);

	int result = fileDialog.exec();
	if (result == QDialog::Accepted)
	{
		// get path
		QString fullPath = fileDialog.selectedFiles()[0];
		String path(fullPath.toUtf8().constData());

		// get category
		String cat = path.ExtractLastDirName();
		String file = path.ExtractFileName();
		file.StripFileExtension();

		// check if we are overwriting ourselves
		if (cat == this->category && file == this->file)
		{
			// same file, use normal save instead
			this->OnSave();
			return;
		}
		// create resource string
		String newAttrResource;
		String newConstResource;
		String newPhyResource;

		// reformat resource
		newAttrResource.Format("src:assets/%s/%s.attributes", cat.AsCharPtr(), file.AsCharPtr());
		newConstResource.Format("src:assets/%s/%s.constants", cat.AsCharPtr(), file.AsCharPtr());
		newPhyResource.Format("src:assets/%s/%s.physics", cat.AsCharPtr(), file.AsCharPtr());

		String attrResource;
		String constResource;
		String phyResource;

		// reformat resource
		attrResource.Format("src:assets/%s/%s.attributes", this->category.AsCharPtr(), this->file.AsCharPtr());
		constResource.Format("src:assets/%s/%s.constants", this->category.AsCharPtr(), this->file.AsCharPtr());
		phyResource.Format("src:assets/%s/%s.physics", this->category.AsCharPtr(), this->file.AsCharPtr());

		// create string of name
		String name = cat + "/" + file;

		// copy original model to new
		IoServer::Instance()->CopyFile(attrResource, newAttrResource);
		IoServer::Instance()->CopyFile(constResource, newConstResource);
		IoServer::Instance()->CopyFile(phyResource, newPhyResource);	

		// create new attributes, constants and physics
		Ptr<ModelAttributes> attrs = ModelDatabase::Instance()->LookupAttributes(name, true);
		Ptr<ModelConstants> consts = ModelDatabase::Instance()->LookupConstants(name, true);
		Ptr<ModelPhysics> phys = ModelDatabase::Instance()->LookupPhysics(name, true);

		// also replace old xml by creating stream
		String lastVersion = this->action->GetLastAttrVersion();
		Ptr<Stream> stream = IoServer::Instance()->CreateStream(newAttrResource);
		stream->SetAccessMode(Stream::WriteAccess);
		stream->Open();
		stream->Write((void*)lastVersion.AsCharPtr(), lastVersion.Length());
		stream->Close();

		// create model builder
		Ptr<ModelBuilder> modelBuilder = ModelBuilder::Create();

		// clear attributes
		attrs->Clear();

		// save attributes again
		attrs->Load(stream);

		// set data for model builder
		modelBuilder->SetAttributes(attrs);
		modelBuilder->SetConstants(consts);
		modelBuilder->SetPhysics(phys);

		// create original resource uri
		String resource;
		resource.Format("mdl:%s/%s.n3", cat.AsCharPtr(), file.AsCharPtr());

		// save model to temporary
		modelBuilder->SaveN3(resource, Platform::Win32);

		// send modification message to remote end
		Ptr<ReloadResourceIfExists> msg = ReloadResourceIfExists::Create();
		msg->SetResourceName(resource);
		QtRemoteInterfaceAddon::QtRemoteClient::GetClient("editor")->Send(msg.upcast<Messaging::Message>());

		// save physics as well
		resource.Format("physics:%s/%s.np3", cat.AsCharPtr(), file.AsCharPtr());
		modelBuilder->SaveN3Physics(resource, Platform::Win32);

		// also send physics update
		msg = ReloadResourceIfExists::Create();
		msg->SetResourceName(resource);
		QtRemoteInterfaceAddon::QtRemoteClient::GetClient("editor")->Send(msg.upcast<Messaging::Message>());

		// emit that the model handler saved a new file
		String res;
		res.Format("mdl:cat/file");
		emit ModelSavedAs(res);

		// generate thumbnail
		String thumbnail = String::Sprintf("src:assets/%s/%s_n3.thumb", this->category.AsCharPtr(), this->file.AsCharPtr());

		// get preview state
		Ptr<PreviewState> previewState = ContentBrowserApp::Instance()->GetPreviewState();
		previewState->SaveThumbnail(thumbnail, true);

		// mark save button
		this->ui->saveButton->setStyleSheet("background-color: rgb(4, 200, 0); color: white");

		// update thumbnail
		this->UpdateModelThumbnail();
	}	
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::OnReconfigure()
{
	// get and open model importer
	ModelImporter::ModelImporterWindow* importer = ContentBrowserApp::Instance()->GetWindow()->GetModelImporter();
	IO::URI path = String::Sprintf("src:assets/%s/%s.fbx", this->category.AsCharPtr(), this->file.AsCharPtr());
	importer->SetUri(path);
	importer->show();
	importer->raise();
	QApplication::processEvents();
	importer->Open();
}

//------------------------------------------------------------------------------
/**
	This function will take the current constants, physics and attributes, compose a model and replace the intermediate one.
	Use this function whenever there is some substantial change to a model node, for example material switch or node addition/removal
*/
void
ModelHandler::MakeModel()
{
	// create model builder
	Ptr<ModelBuilder> modelBuilder = ModelBuilder::Create();

	// set data for model builder
	modelBuilder->SetAttributes(this->attributes);
	modelBuilder->SetConstants(this->constants);
	modelBuilder->SetPhysics(this->physics);

	// create original resource uri
	String resource;
	resource.Format("int:models/%s/%s_temp.n3", this->category.AsCharPtr(), this->file.AsCharPtr());

	// save model to temporary
	modelBuilder->SaveN3(resource, Platform::Win32);

	// reload model, then call OnModelReloaded when its done
	//ContentBrowserApp::Instance()->GetPreviewState()->PreImportModel();
	Ptr<ReloadResourceIfExists> msg = ReloadResourceIfExists::Create();
	msg->SetResourceName(resource);	
	__StaticSend(GraphicsInterface, msg);
	this->OnModelReloaded(msg.upcast<Messaging::Message>());
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::OnModelReloaded(const Ptr<Messaging::Message>& msg)
{
	// if we have a character component, make sure to set all his skins as visible again
	if (0 != this->characterFrame)
	{
		const Ptr<CharacterNodeHandler>& charHandler = this->characterFrame->GetHandler();
		//charHandler->ReshowSkins();
	}

	Ptr<ReloadResourceIfExists> rmsg = msg.downcast<ReloadResourceIfExists>();
	Ptr<PreviewState> previewState = ContentBrowserApp::Instance()->GetPreviewState();
	String gfx = rmsg->GetResourceName();
	previewState->SetModel(gfx);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::SetupTabs()
{
	// format internal model name
	String res;
	res.Format("mdl:%s/%s.n3", this->category.AsCharPtr(), this->file.AsCharPtr());

	// create grid layout for node frame
	QTabWidget* nodeWidget = this->ui->nodeWidget;

	// enable add particle button
	this->ui->addParticleNode->setEnabled(true);

	// remove tabs
	while (nodeWidget->currentWidget())
	{
		QWidget* widget = nodeWidget->widget(0);
		delete widget;
		nodeWidget->removeTab(0);
	}

	// get list of shapes
	const Array<ModelConstants::ShapeNode>& shapes = this->constants->GetShapeNodes();

	if (shapes.Size() > 0)
	{
		// create new frame
		ModelNodeFrame* nodeFrame = new ModelNodeFrame;
		nodeFrame->SetModelHandler(this);
		this->nodeFrames.Append(nodeFrame);
		nodeWidget->addTab(nodeFrame, "Meshes");

		// iterate over shapes and add them to the model node frame
		IndexT i;
		for (i = 0; i < shapes.Size(); i++)
		{
			nodeFrame->AddModelNode(shapes[i].type, shapes[i].name, shapes[i].path, res);
		}
	}


	// get list of characters
	const Array<ModelConstants::CharacterNode>& characters = this->constants->GetCharacterNodes();
	n_assert(characters.Size() <= 1);

	// iterate over shapes and create frames
	IndexT i;
	for (i = 0; i < characters.Size(); i++)
	{
		CharacterNodeFrame* nodeFrame = new CharacterNodeFrame;
		this->characterFrame = nodeFrame;

		nodeFrame->GetHandler()->SetItemHandler(this);
		nodeFrame->GetHandler()->SetName(characters[i].name);
		nodeFrame->GetHandler()->Setup();

		// add frame to tab box
		nodeWidget->addTab(nodeFrame, "Animation");

		// disable add particle button
		this->ui->addParticleNode->setEnabled(false);
	}

	// get list of all skins
	const Array<ModelConstants::Skin>& skins = this->constants->GetSkins();

	if (skins.Size() > 0)
	{
		// create new frame
		ModelNodeFrame* nodeFrame = new ModelNodeFrame;
		nodeFrame->SetModelHandler(this);
		this->nodeFrames.Append(nodeFrame);
		nodeWidget->addTab(nodeFrame, "Meshes");

		// iterate over shapes and add them to the model node frame
		IndexT i;
		for (i = 0; i < skins.Size(); i++)
		{
			// skip skins without fragments
			if (skins[i].skinFragments.IsEmpty())
			{
				continue;
			}

			nodeFrame->AddModelNode(skins[i].type, skins[i].name, skins[i].path, res);
		}
	}

	// get a list of all particles
	const Array<ModelConstants::ParticleNode>& particleNodes = this->constants->GetParticleNodes();
	for (i = 0; i < particleNodes.Size(); i++)
	{
		// create new frame
		ParticleNodeFrame* nodeFrame = new ParticleNodeFrame;
		this->particleFrames.Append(nodeFrame);

		// get state
		const State& state = this->attributes->GetState(particleNodes[i].path);

		// get attrs
		const Particles::EmitterAttrs& attrs = this->attributes->GetEmitterAttrs(particleNodes[i].path);

		// setup handler
		nodeFrame->GetHandler()->SetModelHandler(this);
		nodeFrame->GetHandler()->SetType(particleNodes[i].type);
		nodeFrame->GetHandler()->SetName(particleNodes[i].name);
		nodeFrame->GetHandler()->SetPath(particleNodes[i].path);
		nodeFrame->GetHandler()->Setup(res);

		// add frame to tab box
		nodeWidget->addTab(nodeFrame, "Particle");
	}

	const Array<ModelConstants::PhysicsNode>& physicsNodes = this->constants->GetPhysicsNodes();
	if (physicsNodes.Size() > 0)
	{
		// add physics tab to box
		PhysicsNodeFrame* nodeFrame = new PhysicsNodeFrame;
		nodeWidget->addTab(nodeFrame, "Physics");

		for (i = 0; i < physicsNodes.Size(); i++)
		{
			const ModelConstants::PhysicsNode& node = physicsNodes[i];
			nodeFrame->GetHandler()->AddNode(node.name);
		}
	}
	
}

//------------------------------------------------------------------------------
/**
*/
void
ModelHandler::UpdateModelThumbnail()
{
	String file = this->file;
	file.StripFileExtension();
	String cat = this->category;
	cat.StripAssignPrefix();
	String thumbnail = String::Sprintf("src:assets/%s/%s_n3.thumb", cat.AsCharPtr(), file.AsCharPtr());
	QPixmap pixmap;
	IO::URI texFile = thumbnail;
	pixmap.load(texFile.LocalPath().AsCharPtr());
	pixmap = pixmap.scaled(QSize(67, 67), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	this->ui->modelThumbnail->setPixmap(pixmap);
}

} // namespace Widgets