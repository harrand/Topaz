#ifndef TOPAZ_CORE_BOOLPROXY_HPP
#define TOPAZ_CORE_BOOLPROXY_HPP

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
    
    /**
     * Wrapper class for a simple bool. Used to avoid std::vector<bool> woes.
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