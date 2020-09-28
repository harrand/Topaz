#ifndef TOPAZ_GEO_BOOLPROXY_HPP
#define TOPAZ_GEO_BOOLPROXY_HPP

namespace tz::geo
{
    class BoolProxy
    {
    public:
        BoolProxy(bool b): b(b){}
        operator bool() const{return this->b;}
    private:
        bool b;
    };
}

#endif // TOPAZ_GEO_BOOLPROXY_HPP