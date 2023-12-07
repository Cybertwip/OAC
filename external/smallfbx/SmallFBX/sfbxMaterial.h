#pragma once
#include "sfbxObject.h"

#include <sstream>

namespace sfbx {

// texture & material

// Video represents image data
class Video : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
    ObjectSubClass getSubClass() const override;
	
protected:
	void importFBXObjects() override;
	void exportFBXObjects() override;
	void exportFBXConnections() override;
private:
	std::vector<std::stringstream> mChildStreams;
};

class Texture : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;

protected:
	void importFBXObjects() override;
	void exportFBXObjects() override;
	void exportFBXConnections() override;

private:
	std::vector<std::stringstream> mChildStreams;
};

class Material : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;

protected:
    void importFBXObjects() override;
    void exportFBXObjects() override;
	void exportFBXConnections() override;

private:
	std::vector<std::stringstream> mChildStreams;
};


class Implementation : public Object
    {
using super = Object;
public:
    ObjectClass getClass() const override;
};

class BindingTable : public Object
{
using super = Object;
public:
    ObjectClass getClass() const override;
};

} // namespace sfbx
