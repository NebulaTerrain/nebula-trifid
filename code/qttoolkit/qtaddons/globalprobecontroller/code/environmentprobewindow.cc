//------------------------------------------------------------------------------
//  environmentprobewindow.cc
//  (C) 2012-2014 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "environmentprobewindow.h"
#include "lighting/environmentprobe.h"
#include "io/uri.h"
#include <QFileDialog>
#include "code/assetbrowser.h"

namespace Lighting
{

//------------------------------------------------------------------------------
/**
*/
EnvironmentProbeWindow::EnvironmentProbeWindow()
{
	// setup UI
	this->ui.setupUi(this);
	this->setWindowFlags(Qt::Dialog);

	// connect signals
	connect(this->ui.reflectionMapEdit, SIGNAL(editingFinished()), this, SLOT(OnReflectionChanged()));
	connect(this->ui.irradianceMapEdit, SIGNAL(editingFinished()), this, SLOT(OnIrradianceChanged()));
	
	connect(this->ui.reflectionMapButton, SIGNAL(pressed()), this, SLOT(OnBrowseReflection()));
	connect(this->ui.irradianceMapButton, SIGNAL(pressed()), this, SLOT(OnBrowseIrradiance()));

	// setup text fields
	this->ui.reflectionMapEdit->setText("system/sky_refl");
	this->ui.irradianceMapEdit->setText("system/sky_irr");

	// setup images
	this->OnReflectionChanged();
	this->OnIrradianceChanged();
}

//------------------------------------------------------------------------------
/**
*/
EnvironmentProbeWindow::~EnvironmentProbeWindow()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void
EnvironmentProbeWindow::OnReflectionChanged()
{
	Util::String str = this->ui.reflectionMapEdit->text().toUtf8().constData();
	str.ChangeAssignPrefix("tex");
	str.ChangeFileExtension("dds");

	// assign to default environment probe, quite easy
	EnvironmentProbe::DefaultEnvironmentProbe->AssignReflectionMap(str);

	// update edit with modified value
	this->ui.reflectionMapEdit->blockSignals(true);
	this->ui.reflectionMapEdit->setText(str.AsCharPtr());
	this->ui.reflectionMapEdit->blockSignals(false);

	// now update the icon
	IO::URI uri(str);
	QPixmap img(uri.LocalPath().AsCharPtr());
	img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->ui.reflectionMapIcon->setPixmap(img);
}

//------------------------------------------------------------------------------
/**
*/
void
EnvironmentProbeWindow::OnIrradianceChanged()
{
	Util::String str = this->ui.irradianceMapEdit->text().toUtf8().constData();
	str.ChangeAssignPrefix("tex");
	str.ChangeFileExtension("dds");

	// assign to default environment probe, quite easy
	EnvironmentProbe::DefaultEnvironmentProbe->AssignIrradianceMap(str);

	// update edit with modified value
	this->ui.irradianceMapEdit->blockSignals(true);
	this->ui.irradianceMapEdit->setText(str.AsCharPtr());
	this->ui.irradianceMapEdit->blockSignals(false);

	// now update the icon
	IO::URI uri(str);
	QPixmap img(uri.LocalPath().AsCharPtr());
	img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->ui.irradianceMapIcon->setPixmap(img);
}

//------------------------------------------------------------------------------
/**
*/
void
EnvironmentProbeWindow::OnBrowseReflection()
{
	// exec dialog
	int res = ResourceBrowser::AssetBrowser::Instance()->Execute("Assign to: Global Reflection", ResourceBrowser::AssetBrowser::Textures);
	if (res == QDialog::Accepted)
	{
		// convert to nebula string
		Util::String file = ResourceBrowser::AssetBrowser::Instance()->GetSelectedTexture().toUtf8().constData();

		// get category
		Util::String category = file.ExtractLastDirName();

		// get actual file
		Util::String texFile = file.ExtractFileName();

		// compose strings
		file = category + "/" + texFile;

		// update text field
		this->ui.reflectionMapEdit->setText(file.AsCharPtr());
		this->OnReflectionChanged();
	}
}

//------------------------------------------------------------------------------
/**
*/
void
EnvironmentProbeWindow::OnBrowseIrradiance()
{
	// exec dialog
	int res = ResourceBrowser::AssetBrowser::Instance()->Execute("Assign to: Global Irradiance", ResourceBrowser::AssetBrowser::Textures);
	if (res == QDialog::Accepted)
	{
		// convert to nebula string
		Util::String file = ResourceBrowser::AssetBrowser::Instance()->GetSelectedTexture().toUtf8().constData();

		// get category
		Util::String category = file.ExtractLastDirName();

		// get actual file
		Util::String texFile = file.ExtractFileName();

		// compose strings
		file = category + "/" + texFile;

		// update text field
		this->ui.irradianceMapEdit->setText(file.AsCharPtr());
		this->OnIrradianceChanged();
	}
}

} // namespace Lighting