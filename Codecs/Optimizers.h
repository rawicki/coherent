#ifndef CODECS_OPTIMIZATIONS_H
#define CODECS_OPTIMIZATIONS_H

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/comparison.hpp>
#include <string>


template <typename T>
struct GeneralOptimizer
{
    typedef T optimized_type;

    template <typename Encoder>
    static void encode(Encoder& enc, const T& t)
    {
        enc.memcpy(reinterpret_cast<const char*>(&t), sizeof(T));   //src, 
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& t)
    {
        dec.memcpy(reinterpret_cast<char*>(&t), sizeof(T));
    }
};


template <typename Sequence>
struct SequenceOptimizer
{
    typedef SequenceOptimizer optimized_type;

    template <typename Encoder>
    static void encode(Encoder& enc, const Sequence& seq)
    {
        enc(static_cast<uint32_t>(seq.size()));
        enc.memcpy(reinterpret_cast<const char*>(&(*seq.begin())), sizeof(typename Sequence::value_type) * seq.size());
    }
    template <typename Decoder>
    static void decode(Decoder& dec, Sequence& seq)
    {
        uint32_t x;
        dec(x);
        seq.resize(x);
        dec.memcpy(reinterpret_cast<char*>(&(*seq.begin())), sizeof(typename Sequence::value_type) * seq.size());
    }
};

struct StringZOptimizer
{
    typedef std::string optimized_type;

    template <typename Encoder>
    static void encode(Encoder& enc, const std::string& s)
    {
        enc.memcpy(reinterpret_cast<const char*>(&(*s.begin())), s.size()+1);
    }
    template <typename Decoder>
    static void decoder(Decoder& dec, std::string& s);
    //{
        //najpierw peek - znalezienie '\0', resize, dopiero memcpy...
        //enc.memcpy(static_cast<char*>(), s.size()+1);
    //}
};


//optimization helper

template <
    template <class> class Codecs,
    typename Optimizer,
    typename T
>
struct OptimizationHelper : public Codecs<T>
{
};

template <
    template <class> class Codecs,
    typename Optimizer
>
struct OptimizationHelper<Codecs, Optimizer, typename Optimizer::optimized_type> : public Optimizer
{
};


template <template <class> class Codecs, typename Optimizer>
struct makeCodecWithOptimizationFor
{
    template <typename T>
    struct value : public OptimizationHelper<Codecs, Optimizer, T>
    {
    };
};


//default set of optimizers

template <template <class> class Codecs, typename T>
struct DefaultOptimizerHelper : public Codecs<T>
{
};

template <template <class> class Codecs, typename T>
struct DefaultOptimizerHelper<Codecs, std::vector<T> > : public
    boost::mpl::if_<
        boost::is_integral<T>,
        SequenceOptimizer<std::vector<T> >,
        Codecs<std::vector<T> >
    >::type
{
};

        //consider standard arrays of integers
        //  boost::mpl::if_<
        //      boost::is_array<T>::type,
        //      boost::mpl::if_<
        //          boost::is_integral< boost::remove_extent<T>  >::type
        //      >::type
        //  >::type


template <template <class> class Codecs, typename T>
struct DefaultOptimizer : public
    boost::mpl::if_<
        boost::mpl::and_<
            boost::is_integral<T>,
            boost::mpl::greater_equal<
                boost::mpl::int_<sizeof(T)>,
                boost::mpl::int_<8>
            >
        >,
            GeneralOptimizer<T>,
            typename boost::mpl::if_<
                boost::is_same<T, std::string>,
                SequenceOptimizer<std::string>,
                DefaultOptimizerHelper<Codecs, T>
            >::type
    >::type
{
};


template <template <class> class Codecs>
struct makeCodecWithDefaultOptimizer
{
    //this is a set of optimizers for:
    // * integer types with sizeof not less than 8,
    // * std::vector<integer type>
    // * std::string

    template <typename T>
    struct value : public DefaultOptimizer<Codecs, T>
    {
    };
};


#endif /* CODECS_OPTIMIZATIONS_H */
