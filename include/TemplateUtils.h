// include/TemplateUtils.h
#ifndef TEMPLATEUTILS_H
#define TEMPLATEUTILS_H

// Funcție șablon care aplică un spor de performanță (Atac, HP, Taxe) și plafonează valoarea la un maxim
template <typename T>
T applyStatBonus(T currentVal, T bonus, T maxLimit) {
    T newVal = currentVal + bonus;
    if (newVal > maxLimit) {
        return maxLimit;
    }
    return newVal;
}

#endif // TEMPLATEUTILS_H