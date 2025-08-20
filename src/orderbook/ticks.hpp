#pragma once
#include <binapi/api.hpp>
#include <limits>

// HELPER VAR/FUNCTIONS FOR PRICE/QTY TICK CONVERSION 

using dec = binapi::double_type;

static const dec PRICE_TICK_DEC("0.1"); 
static const dec QTY_STEP_DEC("0.00001");

constexpr int TOPN = 5;

inline Price bid_price_to_ticks(const dec& px)
{
    dec t = px / PRICE_TICK_DEC; 
    return static_cast<Price>(floor(t).convert_to<long long>()); 
}

inline Price ask_price_to_ticks(const dec& px)
{
    dec t = px / PRICE_TICK_DEC;

    return static_cast<Price>(ceil(t).convert_to<long long>()); 
}

inline double ticks_to_price(Price pticks)
{
    return (PRICE_TICK_DEC * dec(pticks)).convert_to<double>(); 

}

inline Quantity qty_to_ticks(const dec& q)
{
    dec ticks = q / QTY_STEP_DEC;

    long long iticks = (ticks + dec("0.5")).convert_to<long long>(); 
    if (iticks < 0)
    {
        iticks = 0; 
    }
    if (iticks > std::numeric_limits<Quantity>::max())
    {
        iticks = std::numeric_limits<Quantity>::max(); 
    }

    return static_cast<Quantity>(iticks);
}

inline double ticks_to_qty(Quantity qticks)
{
    return (QTY_STEP_DEC * dec(qticks)).convert_to<double>(); 
}
