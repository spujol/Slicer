
// MRML includes
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumePropertyNode);

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::vtkMRMLVolumePropertyNode(void)
{
  this->ObservedEvents = vtkIntArray::New();
  this->ObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->ObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->VolumeProperty = NULL;

  vtkVolumeProperty *node  = vtkVolumeProperty::New();
  vtkSetAndObserveMRMLObjectEventsMacro(
    this->VolumeProperty, node, this->ObservedEvents);
  node->Delete();

  // Observe the transfer functions
  this->SetColor(node->GetRGBTransferFunction());
  this->SetScalarOpacity(node->GetScalarOpacity());
  this->SetGradientOpacity(node->GetGradientOpacity());

  this->SetHideFromEditors(0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::~vtkMRMLVolumePropertyNode(void)
{
  if(this->VolumeProperty)
    {
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetScalarOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetGradientOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction());
    vtkSetAndObserveMRMLObjectMacro(this->VolumeProperty, NULL);
    }
  this->ObservedEvents->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  //vtkIndent indent(nIndent);
  of << " interpolation=\"" <<this->VolumeProperty->GetInterpolationType()<< "\"";
  of << " shade=\"" <<this->VolumeProperty->GetShade()<< "\"";
  of << " diffuse=\"" <<this->VolumeProperty->GetDiffuse()<< "\"";
  of << " ambient=\"" <<this->VolumeProperty->GetAmbient()<< "\"";
  of << " specular=\"" <<this->VolumeProperty->GetSpecular()<< "\"";
  of << " specularPower=\"" <<this->VolumeProperty->GetSpecularPower()<<"\"";
  of << " scalarOpacity=\"" << this->GetPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity())  << "\"";
  of << " gradientOpacity=\"" <<this->GetPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity())<< "\"";
  of << " colorTransfer=\"" <<this->GetColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction())<< "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts!=NULL)
    {
    attName= *(atts++);
    attValue= *(atts++);
    if(!strcmp(attName,"scalarOpacity"))
      {
      vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
      this->GetPiecewiseFunctionFromString(attValue,scalarOpacity);
      this->SetScalarOpacity(scalarOpacity);
      scalarOpacity->Delete();
      }
    else if(!strcmp(attName,"gradientOpacity"))
      {
      vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
      this->GetPiecewiseFunctionFromString(attValue,gradientOpacity);
      this->SetGradientOpacity(gradientOpacity);
      gradientOpacity->Delete();
      }
    else if(!strcmp(attName,"colorTransfer"))
      {
      vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
      this->GetColorTransferFunctionFromString(attValue,colorTransfer);
      this->SetColor(colorTransfer);
      colorTransfer->Delete();
      }
    else if(!strcmp(attName,"interpolation"))
      {
      int interpolation;
      std::stringstream ss;
      ss <<attValue;
      ss>>interpolation;
      this->VolumeProperty->SetInterpolationType(interpolation);
      }
    else if(!strcmp(attName,"shade"))
      {
      int shade;
      std::stringstream ss;
      ss <<attValue;
      ss>>shade;
      this->VolumeProperty->SetShade(shade);
      }
    else if(!strcmp(attName,"diffuse"))
      {
      double diffuse;
      std::stringstream ss;
      ss<<attValue;
      ss>>diffuse;
      this->VolumeProperty->SetDiffuse(diffuse);
      }
    else if(!strcmp(attName,"ambient"))
      {
      double ambient;
      std::stringstream ss;
      ss<<attValue;
      ss>>ambient;
      this->VolumeProperty->SetAmbient(ambient);
      }
    else if(!strcmp(attName,"specular"))
      {
      double specular;
      std::stringstream ss;
      ss<<attValue;
      ss>>specular;
      this->VolumeProperty->SetSpecular(specular);
      }
    else if(!strcmp(attName,"specularPower"))
      {
      int specularPower;
      std::stringstream ss;
      ss<<attValue;
      ss>>specularPower;
      this->VolumeProperty->SetSpecularPower(specularPower);
      }//else if
    }//while

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVolumePropertyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  this->CopyParameterSet(anode);
  
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::CopyParameterSet(vtkMRMLNode *anode)
{
  //cast
  vtkMRMLVolumePropertyNode *node = (vtkMRMLVolumePropertyNode *) anode;

  this->VolumeProperty->SetIndependentComponents(
    node->VolumeProperty->GetIndependentComponents());
  this->VolumeProperty->SetInterpolationType(
    node->VolumeProperty->GetInterpolationType());

  //VolumeProperty
  for (int i=0;i<VTK_MAX_VRCOMP;i++)
    {
    this->VolumeProperty->SetComponentWeight(
      i,node->GetVolumeProperty()->GetComponentWeight(i));
    //TODO problem no set method
    // vtkPiecewiseFunction *gray=node->GetVolumeProperty()->GetGrayTransferFunction=());L
    //   this->VolumeProperty->SetGry
    //TODO problem no set ColorChannels Method
    //this->VolumeProperty->SetCGetColorChannels(
    //mapping functions
    vtkColorTransferFunction *rgbTransfer=vtkColorTransferFunction::New();
    rgbTransfer->DeepCopy(
      node->GetVolumeProperty()->GetRGBTransferFunction(i));
    this->SetColor(rgbTransfer, i);
    rgbTransfer->Delete();

    vtkPiecewiseFunction *scalar=vtkPiecewiseFunction::New();
    scalar->DeepCopy(node->GetVolumeProperty()->GetScalarOpacity(i));
    this->SetScalarOpacity(scalar, i);
    scalar->Delete();
    this->VolumeProperty->SetScalarOpacityUnitDistance(
      i,this->VolumeProperty->GetScalarOpacityUnitDistance(i));

    vtkPiecewiseFunction *gradient=vtkPiecewiseFunction::New();
    gradient->DeepCopy(node->GetVolumeProperty()->GetGradientOpacity(i));
    this->SetGradientOpacity(gradient, i);
    gradient->Delete();

    //TODO Copy default gradient?
    this->VolumeProperty->SetDisableGradientOpacity(
      i,node->GetVolumeProperty()->GetDisableGradientOpacity(i));
    this->VolumeProperty->SetShade(i,node->GetVolumeProperty()->GetShade(i));
    this->VolumeProperty->SetAmbient(i, node->VolumeProperty->GetAmbient(i));
    this->VolumeProperty->SetDiffuse(i, node->VolumeProperty->GetDiffuse(i));
    this->VolumeProperty->SetSpecular(i, node->VolumeProperty->GetSpecular(i));
    this->VolumeProperty->SetSpecularPower(
      i, node->VolumeProperty->GetSpecularPower(i));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os<<indent<<"VolumeProperty: ";
  this->VolumeProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  switch (event)
    {
    case vtkCommand::StartEvent:
    case vtkCommand::EndEvent:
    case vtkCommand::StartInteractionEvent:
    case vtkCommand::InteractionEvent:
    case vtkCommand::EndInteractionEvent:
      this->InvokeEvent(event);
      break;
    case vtkCommand::ModifiedEvent:
      this->Modified();
      break;
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode
::dataToString(double* data, int size)
{
  std::stringstream resultStream;
  double *it = data;
  //write header
  resultStream << size;
  for (int i=0; i < size; ++i)
    {
    resultStream << " ";
    resultStream << *it;
    it++;
    }
  return resultStream.str();
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNode
::dataFromString(const std::string& dataString, double* &data)
{
  std::stringstream stream;
  stream << dataString;

  int size=0;
  stream >> size;
  if (size==0)
    {
    return 0;
    }

  data = new double[size];
  for(int i=0; i < size; ++i)
    {
    stream >> data[i];
    }
  return size;
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode
::GetPiecewiseFunctionString(vtkPiecewiseFunction* function)
{
  return vtkMRMLVolumePropertyNode::dataToString(
    function->GetDataPointer(), function->GetSize() * 2);
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode
::GetColorTransferFunctionString(vtkColorTransferFunction* function)
{
  return vtkMRMLVolumePropertyNode::dataToString(
    function->GetDataPointer(), function->GetSize() * 4);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::GetPiecewiseFunctionFromString(
  const std::string& str,vtkPiecewiseFunction* result)
{
  double* data = 0;
  int size = vtkMRMLVolumePropertyNode::dataFromString(str, data);
  if (!size)
    {
    return;
    }
  result->FillFromDataPointer(size/2, data);
  delete [] data;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::GetColorTransferFunctionFromString(
  const std::string& str, vtkColorTransferFunction* result)
{
  double* data = 0;
  int size = vtkMRMLVolumePropertyNode::dataFromString(str, data);
  if (!size)
    {
    return;
    }
  result->FillFromDataPointer(size/4, data);
  delete [] data;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLVolumePropertyNode::CreateDefaultStorageNode()
{
  return vtkMRMLVolumePropertyStorageNode::New();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::SetScalarOpacity(vtkPiecewiseFunction* newScalarOpacity, int component)
{
  vtkUnObserveMRMLObjectMacro(
    this->VolumeProperty->GetScalarOpacity(component));

  this->VolumeProperty->SetScalarOpacity(component, newScalarOpacity);

  vtkObserveMRMLObjectEventsMacro(
    this->VolumeProperty->GetScalarOpacity(component),
    this->ObservedEvents);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::SetGradientOpacity(vtkPiecewiseFunction* newGradientOpacity, int component)
{
  vtkUnObserveMRMLObjectMacro(
    this->VolumeProperty->GetGradientOpacity(component));

  this->VolumeProperty->SetGradientOpacity(component, newGradientOpacity);

  vtkObserveMRMLObjectEventsMacro(
    this->VolumeProperty->GetGradientOpacity(component),
    this->ObservedEvents);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::SetColor(vtkColorTransferFunction* newColorFunction, int component)
{
  vtkUnObserveMRMLObjectMacro(
    this->VolumeProperty->GetRGBTransferFunction(component));

  this->VolumeProperty->SetColor(component, newColorFunction);

  vtkObserveMRMLObjectEventsMacro(
    this->VolumeProperty->GetRGBTransferFunction(component),
    this->ObservedEvents);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumePropertyNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->VolumeProperty &&
     this->VolumeProperty->GetMTime() > this->GetStoredTime());
}
