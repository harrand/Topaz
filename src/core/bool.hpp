#ifndef TOPAZ_CORE_BOOLPROXY_HPP
#define TOPAZ_CORE_BOOLPROXY_HPP

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz::core)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    
    class BoolProxy
    {
    public:
        BoolProxy(bool b): b(b){}
        operator bool() const{return this->b;}
    private:
        bool b;
    };

    /**
     * @}
     */
}

#endif // TOPAZ_CORE_BOOLPROXY_HPP