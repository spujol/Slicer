#ifndef __qSlicerVolumeRenderingModule_h
#define __qSlicerVolumeRenderingModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModulePrivate;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerVolumeRenderingModule(QObject *parent=0);
  virtual ~qSlicerVolumeRenderingModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help of the module
  virtual QString helpText()const;
  /// Acknowledgement for the module
  virtual QString acknowledgementText()const;
  /// Contributors of the module.
  virtual QStringList contributors()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  virtual QStringList categories()const;

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerVolumeRenderingModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModule);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModule);

};

#endif
