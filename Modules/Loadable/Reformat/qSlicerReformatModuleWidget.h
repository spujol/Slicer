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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerReformatModuleWidget_h
#define __qSlicerReformatModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerReformatModuleExport.h"

class qSlicerReformatModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Reformat
class Q_SLICER_QTMODULES_REFORMAT_EXPORT
qSlicerReformatModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerReformatModuleWidget(QWidget *parent=0);
  virtual ~qSlicerReformatModuleWidget();

  enum OriginReferenceType {ONPLANE, INVOLUME};
  enum AxesReferenceType {axisX=0, axisY, axisZ};

  void setSliceNormal(double x, double y, double z);
  void setSliceNormal(double normal[3]);

protected:
  virtual void setup();

public slots:
  /// Set slice \a offset. Used to set a single value.
  void setSliceOffsetValue(double offset);

  /// Recenter the active node given its coordinates reference.
  void centerSliceNode();

  /// Set slice normal to the camera.
  void setNormalToCamera();

  /// Set the normal to a x axis
  void setNormalToAxisX();

  /// Set the normal to a y axis
  void setNormalToAxisY();

  /// Set the normal to a z axis
  void setNormalToAxisZ();

protected slots:
  /// Triggered upon MRML transform node updates
  void onMRMLSliceNodeModified(vtkObject* caller);

  /// Set slice offset. Used when events will come is rapid succession.
  void onTrackSliceOffsetValueChanged(double offset);

  void onNodeSelected(vtkMRMLNode* node);
  void onSliceVisibilityChanged(bool visible);
  void onReformatWidgetVisibilityChanged(bool visible);
  void onLockReformatWidgetToCamera(bool lock);

  void onOriginCoordinateReferenceButtonPressed(int reference);
  void onWorldPositionChanged();

  void onSliceNormalChanged();
  void onSliceNormalToAxisChanged(AxesReferenceType axis);
  void onSliceOrientationChanged(const QString& orientation);
  void onSliderRotationChanged(double rotationX);

protected:
  QScopedPointer<qSlicerReformatModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerReformatModuleWidget);
  Q_DISABLE_COPY(qSlicerReformatModuleWidget);
};

#endif
