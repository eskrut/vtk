/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkShaderProgram - a glsl shader program
// .SECTION Description
// This class contains the vertex, fragment, geometry shaders that combine to make a shader program
#ifndef __vtkShaderProgram_h
#define __vtkShaderProgram_h

#include "vtkRenderingOpenGL2Module.h" // for export macro
#include "vtkObject.h"

#include <string> // For member variables.
#include <map>    // For member variables.

class vtkMatrix3x3;
class vtkMatrix4x4;
class vtkShader;
class VertexArrayObject;
class vtkWindow;

/**
 * @brief The ShaderProgram uses one or more Shader objects.
 *
 * This class creates a Vertex or Fragment shader, that can be attached to a
 * ShaderProgram in order to render geometry etc.
 */

class VTKRENDERINGOPENGL2_EXPORT vtkShaderProgram : public vtkObject
{
public:
  static vtkShaderProgram *New();
  vtkTypeMacro(vtkShaderProgram, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the vertex shader for this program
  void SetVertexShader(vtkShader *VS);
  vtkGetObjectMacro(VertexShader, vtkShader);

  // Description:
  // Set/Get the fragment shader for this program
  void SetFragmentShader(vtkShader *FS);
  vtkGetObjectMacro(FragmentShader, vtkShader);

  // Description:
  // Set/Get the geometry shader for this program
  void SetGeometryShader(vtkShader *GS);
  vtkGetObjectMacro(GeometryShader, vtkShader);

  // Description:
  // Set/Get flag for if this program is compiled
  vtkGetMacro(Compiled, bool);
  vtkSetMacro(Compiled, bool);
  vtkBooleanMacro(Compiled, bool);

  // Description:
  // Compile this shader program and attached shaders
  virtual int CompileShader();

  // Description:
  // Set/Get the md5 hash of this program
  std::string GetMD5Hash() const { return this->MD5Hash; }
  void SetMD5Hash(const std::string &hash) { this->MD5Hash = hash; }


  /** Options for attribute normalization. */
  enum NormalizeOption {
    /// The values range across the limits of the numeric type.
    /// This option instructs the rendering engine to normalize them to
    /// the range [0.0, 1.0] for unsigned types, and [-1.0, 1.0] for signed
    /// types.
    /// For example, unsigned char values will be mapped so that 0 = 0.0,
    /// and 255 = 1.0.
    /// The resulting floating point numbers will be passed into
    /// the shader program.
    Normalize,
    /// The values should be used as-is. Do not perform any normalization.
    NoNormalize
  };


  /**
   * Attach the supplied shader to this program.
   * @note A maximum of one Vertex shader and one Fragment shader can be
   * attached to a shader program.
   * @return true on success.
   */
  bool AttachShader(const vtkShader *shader);

  /** Detach the supplied shader from this program.
   * @note A maximum of one Vertex shader and one Fragment shader can be
   * attached to a shader program.
   * @return true on success.
   */
  bool DetachShader(const vtkShader *shader);

  /**
   * Attempt to link the shader program.
   * @return false on failure. Query error to get the reason.
   * @note The shaders attached to the program must have been compiled.
   */
  bool Link();

  /**
   * Bind the program in order to use it. If the program has not been linked
   * then link() will be called.
   */
  bool Bind();

  /**
   * Check if the program is currently bound, or not.
   * @return True if the program is bound, false otherwise.
   */
  bool isBound() const { return this->Bound; }

  /** Releases the shader program from the current context. */
  void Release();

  // Description:
  // release any graphics resources this class is using.
  void ReleaseGraphicsResources(vtkWindow *win);

  /** Get the handle of the shader program. */
  int GetHandle() const { return Handle; }

  /** Get the error message (empty if none) for the shader program. */
  std::string GetError() const { return Error; }

  /**
   * Enable the named attribute array. Return false if the attribute array is
   * not contained in the linked shader program.
   */
  bool EnableAttributeArray(const std::string &name);

  /**
   * Disable the named attribute array. Return false if the attribute array is
   * not contained in the linked shader program.
   */
  bool DisableAttributeArray(const std::string &name);

  /**
   * Use the named attribute array with the bound BufferObject.
   * @param name of the attribute (as seen in the shader program).
   * @param offset into the bound BufferObject.
   * @param stride The stride of the element access (i.e. the size of each
   * element in the currently bound BufferObject). 0 may be used to indicate
   * tightly packed data.
   * @param elementType Tag identifying the memory representation of the
   * element.
   * @param elementTupleSize The number of elements per vertex (e.g. a 3D
   * position attribute would be 3).
   * @param normalize Indicates the range used by the attribute data.
   * See NormalizeOption for more information.
   * @return false if the attribute array does not exist.
   */
  bool UseAttributeArray(const std::string &name, int offset, size_t stride,
                         int elementType, int elementTupleSize,
                         NormalizeOption normalize);

  /**
   * Upload the supplied array of tightly packed values to the named attribute.
   * BufferObject attributes should be preferred and this may be removed in
   * future.
   *
   * @param name Attribute name
   * @param array Container of data. See note.
   * @param tupleSize The number of elements per vertex, e.g. a 3D coordinate
   * array will have a tuple size of 3.
   * @param  normalize Indicates the range used by the attribute data.
   * See NormalizeOption for more information.
   *
   * @note The T type must have tightly packed values of
   * T::value_type accessible by reference via T::operator[].
   * Additionally, the standard size() and empty() methods must be implemented.
   * The std::vector classes is an example of such a container.
   */
  template <class T>
  bool SetAttributeArray(const std::string &name, const T &array,
                         int tupleSize, NormalizeOption normalize);

  /** Set the @p name uniform value to int @p v. */
  bool SetUniformi(const std::string &name, int v);
  bool SetUniformf(const std::string &name, float v);
  bool SetUniform2i(const std::string &name, const int v[2]);
  bool SetUniform2f(const std::string &name, const float v[2]);
  bool SetUniform3f(const std::string &name, const float v[3]);
  bool SetUniform4f(const std::string &name, const float v[4]);
  bool SetUniform3uc(const std::string &name, const unsigned char v[3]); // maybe remove
  bool SetUniform4uc(const std::string &name, const unsigned char v[4]); // maybe remove
  bool SetUniformMatrix(const std::string &name, vtkMatrix3x3 *v);
  bool SetUniformMatrix(const std::string &name, vtkMatrix4x4 *v);

  /** Set the @p name uniform array to @p f with @p count elements */
  bool SetUniform1iv(const std::string &name, const int count, const int *f);
  bool SetUniform2iv(const std::string &name, const int count, const int *f);
  bool SetUniform3uv(const std::string &name, const int count, const unsigned char *f);
  bool SetUniform1fv(const std::string &name, const int count, const float *f);
  bool SetUniform3fv(const std::string &name, const int count, const float (*f)[3]);

protected:
  vtkShaderProgram();
  ~vtkShaderProgram();

  vtkShader *VertexShader;
  vtkShader *FragmentShader;
  vtkShader *GeometryShader;

  // hash of the shader program
  std::string MD5Hash;

  bool SetAttributeArrayInternal(const std::string &name, void *buffer,
                                 int type, int tupleSize,
                                 NormalizeOption normalize);
  int Handle;
  int VertexShaderHandle;
  int FragmentShaderHandle;

  bool Linked;
  bool Bound;
  bool Compiled;

  std::string Error;

  std::map<std::string, int> Attributes;

  friend class VertexArrayObject;

private:
  int FindAttributeArray(const std::string &name);
  int FindUniform(const std::string &name);

  vtkShaderProgram(const vtkShaderProgram&);  // Not implemented.
  void operator=(const vtkShaderProgram&);  // Not implemented.
};


#endif
