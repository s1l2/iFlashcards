#include "stdafx.h"
#include "model.h"
#include <boost/random/uniform_real_distribution.hpp>

CardsStorage::TSearchIterator CardsStorage::GetRandomElem( Lang::T lng )
{
  boost::random::uniform_real_distribution<> dist( 0.0, factorSum[lng] );
  double randValue = dist(m_rng);
  TIterator it = m_flashCards.begin();
  
  for( ; it != m_flashCards.end() && it->factor[lng] <= randValue; ++it )
    randValue -= it->factor[lng];  
  
  return TSearchIterator(it);
}

void CardsStorage::ChangeFactor( TIterator it, const Settings &st, Answer::T answer )
{
  const double oldFactor = it->factor[st.Language()];
 
  // Maple:
  //   restart;
  //   Digits := 20:
  //   x1 := 3:
  //   x2 := 10:
  //   m := 50:
  //   for i while i < 100 do
  //   print( i, x1, x2 );
  //     x1 := x1 + (m - x1) * 0.5:
  //     x2 := x2 - 0.2 * (x2 - 1):  
  //   end do:
  const double newFactor = ( answer == Answer::Incorrect ) ?
    //���� ��� ��� ������������ �����, �� ����� ������� ������ � ������� ���������� �����������
    //� ����������� �� ����, ��������� ��� ����������� ���� � ������ ������.
    //�.�. �� ���������� ��������, ����� ������������ ������ ����� ����� ��� ����� �� ���������� �
    //�� ��� ����� 
    oldFactor + ( st.MaxWeight(GetCardsSize()) - oldFactor ) * st.IncorrectAnswerFactor():
    
    //���� ��� ��� ���������� �����, �� �� ������ ���������� ��������� � ������� � 
    //��� ����, ��� ����� ����� ��, ��� ���������� �����, ������� �� ������� ��� ����������
    //� ������� ������ ���� ���������� ��� ������������ ��� ��� �������� oldFactor 
    //������ 2 ��� � ��� 10000
    oldFactor - st.CorrectAnswerFactor() * ( oldFactor - 1 );
    
  factorSum[st.Language()] += newFactor - oldFactor;
  it->factor[st.Language()] = newFactor;
  it->attempts[st.Language()] += 1;
}

CardsStorage::CardsStorage():
  m_rng( static_cast<unsigned int>(std::time(0)) ) 
{
  Clear();
}

double CardsStorage::Score( const Settings &st ) const
{
  if( IsCardsEmpty() )
    return 0.0;

  const double avg = factorSum[st.Language()] / GetCardsSize();
  return std::floor( (st.MaxWeight( GetCardsSize() ) - avg) * 100 + 0.5 ); 
}

void CardsStorage::CorrectWeight()
{
  for( TIterator it = m_flashCards.begin(); it != m_flashCards.end(); ++it )
  {
    if( it->attempts[Lang::Native] == 0 && it->factor[Lang::Native] != it->factor[Lang::Foreign] )
      it->attempts[Lang::Native] = 1;  
  }
}