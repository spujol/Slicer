/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerVolumesIO.h"
#include "qSlicerVolumesIOOptionsWidget.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

//-----------------------------------------------------------------------------
class qSlicerVolumesIOPrivate
{
  public:
  vtkSmartPointer<vtkSlicerVolumesLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerVolumesIO::qSlicerVolumesIO(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesIOPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesIO::qSlicerVolumesIO(vtkSlicerVolumesLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesIOPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerVolumesIO::~qSlicerVolumesIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumesIO::setLogic(vtkSlicerVolumesLogic* logic)
{
  Q_D(qSlicerVolumesIO);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerVolumesLogic* qSlicerVolumesIO::logic()const
{
  Q_D(const qSlicerVolumesIO);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesIO::description()const
{
  return "Volume";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVolumesIO::fileType()const
{
  return qSlicerIO::VolumeFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesIO::extensions()const
{
  // pic files are bio-rad images (see itkBioRadImageIO)
  return QStringList()
    << "Volume (*.hdr *.nhdr *.nrrd *.mhd *.mha *.vti *.nii *.gz *.mgz *.img *.pic)"
    << "Dicom (*.dcm *.ima)"
    << "Image (*.png *.tif *.tiff *.jpg *.jpeg)"
    << "All Files (*)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerVolumesIO::options()const
{
  return new qSlicerVolumesIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerVolumesIO::load(const IOProperties& properties)
{
  Q_D(qSlicerVolumesIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  int options = 0;
  if (properties.contains("labelmap"))
    {
    options |= properties["labelmap"].toBool() ? 0x1 : 0x0;
    }
  if (properties.contains("center"))
    {
    options |= properties["center"].toBool() ? 0x2 : 0x0;
    }
  if (properties.contains("singleFile"))
    {
    options |= properties["singleFile"].toBool() ? 0x4 : 0x0;
    }
  if (properties.contains("autoWindowLevel"))
    {
    options |= properties["autoWindowLevel"].toBool() ? 0x8: 0x0;
    }
  if (properties.contains("discardOrientation"))
    {
    options |= properties["discardOrientation"].toBool() ? 0x10 : 0x0;
    }
  vtkSmartPointer<vtkStringArray> fileList;
  if (properties.contains("fileNames"))
    {
    fileList = vtkSmartPointer<vtkStringArray>::New();
    foreach(QString file, properties["fileNames"].toStringList())
      {
      fileList->InsertNextValue(file.toLatin1());
      }
    }
  Q_ASSERT(d->Logic);
  vtkMRMLVolumeNode* node = d->Logic->AddArchetypeVolume(
    fileName.toLatin1(),
    name.toLatin1(),
    options,
    fileList.GetPointer());
  if (node)
    {
    vtkSlicerApplicationLogic* appLogic =
      d->Logic->GetApplicationLogic();
    vtkMRMLSelectionNode* selectionNode =
      appLogic ? appLogic->GetSelectionNode() : 0;
    if (selectionNode)
      {
      if (vtkMRMLScalarVolumeNode::SafeDownCast(node) &&
          vtkMRMLScalarVolumeNode::SafeDownCast(node)->GetLabelMap())
        {
        selectionNode->SetReferenceActiveLabelVolumeID(node->GetID());
        }
      else
        {
        selectionNode->SetReferenceActiveVolumeID(node->GetID());
        }
      if (appLogic)
        {
        appLogic->PropagateVolumeSelection();
        // TODO: slices should probably be fitting automatically..
        appLogic->FitSliceToAll();
        }
      }
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
