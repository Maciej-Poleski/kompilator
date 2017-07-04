#ifndef KOMPILATOR_WITHRESULT_HXX
#define KOMPILATOR_WITHRESULT_HXX


template<typename T>
class WithResult
{
public:
    T&& getResult();

protected:
    void setResult();

private:
    T result;
};


#endif //KOMPILATOR_WITHRESULT_HXX
