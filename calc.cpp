// Define this to enable debugging
// #define BOOST_SPIRIT_QI_DEBUG

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <string>

namespace ast
{
    struct Nil;

    struct Signed;

    struct Expression;

    struct RightAssocExpr;

    struct Arg;

    struct FunctionCall;

    typedef boost::variant<
            Nil
          , double
          , boost::recursive_wrapper< Signed >
          , boost::recursive_wrapper< Expression >
          , boost::recursive_wrapper< RightAssocExpr >
          , boost::recursive_wrapper< Arg >
          , boost::recursive_wrapper< FunctionCall >
        >
    Operand;

    struct Nil
    {
    };

    inline std::ostream & operator<<( std::ostream & out, Nil ) { out << "nil"; return out; }

    struct Signed
    {
        char sign_;
        Operand operand_;
    };

    struct Arg
    {
        char digit_;
    };

    struct RightAssocExpr
    {
        Operand left_;
        boost::optional< Operand > right_;
    };

    struct FunctionCall
    {
        std::string name_;
        Operand arg_;
    };

    struct Operation
    {
        std::string operator_;
        Operand operand_;
    };

    struct Expression
    {
        Operand head_;
        std::list< Operation > tail_;
    };

}

BOOST_FUSION_ADAPT_STRUCT(
    ast::Signed,
    ( char, sign_ )
    ( ast::Operand, operand_ )
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::Arg,
    ( char, digit_ )
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::RightAssocExpr,
    ( ast::Operand, left_ )
    ( boost::optional< ast::Operand >, right_ )
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::FunctionCall,
    ( std::string, name_ )
    ( ast::Operand, arg_ )
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::Operation,
    ( std::string, operator_ )
    ( ast::Operand, operand_ )
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::Expression,
    ( ast::Operand, head_ )
    ( std::list< ast::Operation >, tail_ )
)

namespace ast
{
    struct Calc
    {
    public:

        typedef double result_type;

    private:

        template< typename... TDoubles >
        void assign( double * args, int index, double head, TDoubles &&... tail )
        {
            args[ index ] = head;
            assign( args, index+1, std::forward< TDoubles >( tail )... );
        }

        void assign( double * args, int index )
        {
        }

    public:

        template< typename... Doubles >
        Calc( Doubles &&... ds )
        {
            static_assert( sizeof...( Doubles ) < 10, "sizeof...(Doubles) has to be less then 10" );
            args_[ 0 ] = sizeof...( Doubles );

            assign( & args_[ 0 ], 1, std::forward< Doubles >( ds )... );
        }

        double operator()( Nil ) const
        {
            BOOST_ASSERT( 0 ); return 0;
        }

        double operator()( double n ) const
        {
            return n;
        }

        double operator()( Operation const & x, double lhs ) const
        {
            if( x.operator_ == "&&" )
            {
                if( fabs( lhs ) < 0.000001 ){
                    return 0;
                }
                else{
                    return boost::apply_visitor( *this, x.operand_ );
                }
            }

            if( x.operator_ == "||" )
            {
                if( fabs( lhs ) < 0.000001 ){
                    return boost::apply_visitor( *this, x.operand_ );
                }
                else{
                    return lhs;
                }
            }

            double const rhs = boost::apply_visitor( *this, x.operand_ );

            if( x.operator_ == "+" ){
                return lhs + rhs;
            }

            if( x.operator_ == "-" ){
                return lhs - rhs;
            }

            if( x.operator_ == "*" ){
                return lhs * rhs;
            }

            if( x.operator_ == "/" ){
                return lhs / rhs;
            }

            if( x.operator_ == "<" ){
                return lhs < rhs;
            }

            if( x.operator_ == "<=" ){
                return lhs <= rhs;
            }

            if( x.operator_ == ">" ){
                return lhs > rhs;
            }

            if( x.operator_ == ">=" ){
                return lhs >= rhs;
            }

            if( x.operator_ == "==" ){
                return lhs == rhs;
            }

            if( x.operator_ == "!=" ){
                return lhs != rhs;
            }

            return 0;
        }

        double operator()( Signed const & x ) const
        {
            double const rhs = boost::apply_visitor( *this, x.operand_ );

            switch( x.sign_ )
            {
                case '-':
                    return - rhs;

                case '+':
                    return + rhs;

                case '!':
                    return ( fabs( rhs ) < 0.000001 ) ? 1 : 0;

                default:
                    BOOST_ASSERT( 0 );
            }
        }

        double operator()( Arg const & x ) const
        {
            return args_[ x.digit_ - '0' ];
        }

        double operator()( RightAssocExpr const & x ) const
        {
            double const base = boost::apply_visitor( *this, x.left_ );

            if( ! x.right_ ){
                return base;
            }
            
            double const exp = boost::apply_visitor( *this, x.right_.get() );

            return std::pow( base, exp );
        }

        double operator()( FunctionCall const & x ) const
        {
            if( x.name_ == "pi" ){
                return M_PI;
            }
            
            if( x.name_ == "e" ){
                return std::exp( 1.0 );
            }

            double const arg = boost::apply_visitor( *this, x.arg_ );

            if( x.name_ == "sin" ){
                return sin( arg );
            }
            
            if( x.name_ == "cos" ){
                return cos( arg );
            }
            
            if( x.name_ == "tan" ){
                return tan( arg );
            }
            
            if( x.name_ == "abs" ){
                return fabs( arg );
            }
            
            if( x.name_ == "rad" ){
                return arg * 2 * M_PI / 360;
            }

            if( x.name_ == "deg" ){
                return arg * 360 / 2 / M_PI;
            }

            if( x.name_ == "log" ){
                return log( arg );
            }

            if( x.name_ == "log10" ){
                return log10( arg );
            }

            if( x.name_ == "log2" ){
                return log2( arg );
            }

            return 0;
        }

        double operator()( Expression const & x ) const
        {
            double state = boost::apply_visitor( *this, x.head_ );

            for( Operation const & oper : x.tail_ )
            {
                state = ( *this )( oper, state );
            }

            return state;
        }
    
    private:

        double args_[ 10 ];
    };
}

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
using boost::phoenix::function;

struct error_handler_
{
    template< typename, typename, typename >
    struct result { typedef void type; };

    template< typename TIterator >
    void operator()(
        qi::info const & what
      , TIterator err_pos, TIterator last ) const
    {
        std::cout
            << "Error! Expecting "
            << what
            << " here: \""
            << std::string( err_pos, last )
            << "\""
            << std::endl
        ;
    }
};

function< error_handler_ > const error_handler = error_handler_();

template< typename TIterator >
struct Calculator
    : qi::grammar< TIterator, ast::Expression(), ascii::space_type >
{
    Calculator()
        : Calculator::base_type( expression )
    {
        qi::char_type char_;
        qi::digit_type digit_;
        qi::double_type double_;
        qi::lit_type lit_;
        qi::string_type string_;
        qi::alpha_type alpha_;
        qi::alnum_type alnum_;

        using qi::on_error;
        using qi::fail;
        using qi::lexeme;

        expression =
            relationalExpression
            >> *(   (string_( "&&" ) > relationalExpression )
                |   (string_( "||" ) > relationalExpression )
                )
            ;

        relationalExpression =
            additiveExpression
            >> *(   (string_( "<=" ) > additiveExpression )
                |   (string_( "<" ) > additiveExpression )
                |   (string_( ">=" ) > additiveExpression )
                |   (string_( ">" ) > additiveExpression )
                |   (string_( "!=" ) > additiveExpression )
                |   (string_( "==" ) > additiveExpression )
                )
            ;

        additiveExpression =
            multiplicativeExpression
            >> *(   (char_( '+' ) > multiplicativeExpression )
                |   (char_( '-' ) > multiplicativeExpression )
                )
            ;

        multiplicativeExpression =
            exponentialExpression
            >> *(   ( char_( '*' ) > exponentialExpression )
                |   ( char_( '/' ) > exponentialExpression )
                )
            ;

        exponentialExpression =
            unaryExpression
            >> -( '^' >> exponentialExpression );
        
        unaryExpression =
                ( char_( '-' ) > unaryExpression )
            |   ( char_( '+' ) > unaryExpression )
            |   ( char_( '!' ) > unaryExpression )
            |   primaryExpression
            ;

        primaryExpression =
                double_
            |   arg
            |   functionCall
            |   '(' > expression > ')'
            ;

        arg =
            ( '_' > digit_ )
            ;

        functionCall =
            ( +alnum_ > '(' > -unaryExpression > ')' )
            ;

        BOOST_SPIRIT_DEBUG_NODE( expression );
        BOOST_SPIRIT_DEBUG_NODE( relationalExpression );
        BOOST_SPIRIT_DEBUG_NODE( additiveExpression );
        BOOST_SPIRIT_DEBUG_NODE( multiplicativeExpression );
        BOOST_SPIRIT_DEBUG_NODE( exponentialExpression );
        BOOST_SPIRIT_DEBUG_NODE( unaryExpression );
        BOOST_SPIRIT_DEBUG_NODE( primaryExpression );
        BOOST_SPIRIT_DEBUG_NODE( arg );
        BOOST_SPIRIT_DEBUG_NODE( functionCall );

        on_error< fail >(
            expression,
            error_handler( qi::_4_type(), qi::_3_type(), qi::_2_type() )
        );
    }

private:

    qi::rule< TIterator, ast::Expression(), ascii::space_type > expression;
    qi::rule< TIterator, ast::Expression(), ascii::space_type > relationalExpression;
    qi::rule< TIterator, ast::Expression(), ascii::space_type > additiveExpression;
    qi::rule< TIterator, ast::Expression(), ascii::space_type > multiplicativeExpression;
    qi::rule< TIterator, ast::RightAssocExpr(), ascii::space_type > exponentialExpression;
    qi::rule< TIterator, ast::Operand(), ascii::space_type > unaryExpression;
    qi::rule< TIterator, ast::Operand(), ascii::space_type > primaryExpression;
    qi::rule< TIterator, ast::Arg(), ascii::space_type > arg;
    qi::rule< TIterator, ast::FunctionCall(), ascii::space_type > functionCall;
};

template< typename... TDoubles>
double calc(
    std::string const & expr,
    TDoubles... ds
)
{
    std::string::const_iterator iter = expr.begin();
    std::string::const_iterator end = expr.end();

    Calculator< std::string::const_iterator > calc;
    
    boost::spirit::ascii::space_type space;

    ast::Expression expression;

    bool const r = phrase_parse( iter, end, calc, space, expression );

    if( r && iter == end )
    {
        ast::Calc calc( std::forward< TDoubles >( ds )... );
        return calc( expression );
    }
    
    throw std::runtime_error( "Parsing failed: " + std::string( iter, end ) );
}

void test()
{
    assert( calc( "1+2*3" ) == 7 );
    assert( calc( "2^3^2" ) == 512 );
    assert( calc( "(1+2)*(3+4)" ) == 21 );

    assert( calc( "1+-1" ) == 0 );
    assert( calc( "1--1" ) == 2 );

    assert( calc( "4/2" ) == 2 );
    assert( calc( "4.5/2" ) == 2.25 );
    assert( calc( "5/2" ) == 2.5 );

    assert( calc( "!1" ) == 0 );
    assert( calc( "!0" ) == 1 );

    assert( calc( "pi()" ) == double(M_PI) );
    assert( calc( "e()" ) == std::exp( 1.0 ) );

    assert( calc( "12||(1/0)" ) == 12 );
    assert( calc( "0&&(1/0)" ) == 0 );
    assert( calc( "1&&2" ) == 2 );
    assert( calc( "2||1" ) == 2 );
    assert( calc( "0||2" ) == 2 );

    assert( calc( "1<2" ) == 1 );
    assert( calc( "2<1" ) == 0 );

    assert( calc( "1>2" ) == 0 );
    assert( calc( "2>1" ) == 1 );

    assert( calc( "1<=2" ) == 1 );
    assert( calc( "2<=2" ) == 1 );
    assert( calc( "3<=2" ) == 0 );

    assert( calc( "1>=2" ) == 0 );
    assert( calc( "2>=2" ) == 1 );
    assert( calc( "3>=2" ) == 1 );

    assert( calc( "abs(1)" ) == 1 );
    assert( calc( "abs(-1)" ) == 1 );

    assert( calc( "sin(rad(0))" ) == 0 );
    assert(( fabs( calc( "sin(rad(90))" ) - 1.0 ) < 0.000001 ));

    assert( calc( "cos(rad(0))" ) == 1 );
    assert(( fabs( calc( "cos(rad(90))" ) - 0.0 ) < 0.0000001 ));

    assert( calc( "log10(1)" ) == 0 );
    assert( calc( "log10(10)" ) == 1 );
    assert( calc( "log10(100)" ) == 2 );

    assert( calc( "log2(1)" ) == 0 );
    assert( calc( "log2(2)" ) == 1 );
    assert( calc( "log2(4)" ) == 2 );

    assert( calc( "_1+_2+_3+_4+_5+_6+_7+_8+_9", 1, 2, 3, 4, 5, 6, 7, 8, 9 ) == 45 );
    assert( calc( "_0" ) == 0 );
    assert( calc( "_0", 1 ) == 1 );
    assert( calc( "_0", 1, 2 ) == 2 );
    assert( calc( "_0", 1, 2, 3 ) == 3 );
}

int main( int argc, char* argv[] )
{
    test();

    using std::cout;
    using std::endl;
    using std::stof;

    try
    {
        if( argc == 2 ){
            cout << calc( argv[1] ) << endl;
        }
        else if( argc == 3 ){
            cout << calc( argv[1], stof(argv[2]) ) << endl;
        }
        else if( argc == 4 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]) ) << endl;
        }
        else if( argc == 5 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]) ) << endl;
        }
        else if( argc == 6 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]) ) << endl;
        }
        else if( argc == 7 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]), stof(argv[6]) ) << endl;
        }
        else if( argc == 8 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]), stof(argv[6]),
                stof(argv[7]) ) << endl;
        }
        else if( argc == 9 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]), stof(argv[6]),
                stof(argv[7]), stof(argv[8]) ) << endl;
        }
        else if( argc == 10 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]), stof(argv[6]),
                stof(argv[7]), stof(argv[8]), stof(argv[9]) ) << endl;
        }
        else if( argc == 11 ){
            cout << calc( argv[1], stof(argv[2]), stof(argv[3]),
                stof(argv[4]), stof(argv[5]), stof(argv[6]),
                stof(argv[7]), stof(argv[8]), stof(argv[9]),
                stof(argv[10]) ) << endl;
        }
    }
    catch( std::runtime_error const & e ){
        std::cerr << e.what() << std::endl;
    }
}

