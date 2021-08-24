#pragma once


class IDeflectionStrategy
{
public:
	virtual void Deflect(float &bulletHeading) = 0;
	virtual void Destroy() = 0;
};
