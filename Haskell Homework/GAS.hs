{-# OPTIONS_GHC -Wall #-}
{-# LANGUAGE MultiParamTypeClasses,
             TypeSynonymInstances, FlexibleInstances #-}

module GAS where

import ProblemState
import Data.Char

import qualified Data.Map.Strict as M

{-
    Pozițiile tablei de joc, în formă (linie, coloană), unde ambele coordonate
    pot fi negative.
-}
type Position = (Int, Int)

{-
    Culorile pătratelor și cercurilor.
-}
data Color = Red | Blue | Gray
    deriving (Eq, Ord, Show)

{-
    Orientările pătratelor și săgeților.
-}
data Heading = North | South | East | West
    deriving (Eq, Ord)

instance Show Heading where
    show North = "^"
    show South = "v"
    show East  = ">"
    show West  = "<"

data Object = Square Color Heading Position | Circle Color Position | Arrow Heading Position
    deriving (Eq, Ord)


emptyLevel :: Level
emptyLevel = Level (M.fromList [])



instance Show Object where
    show (Square c h _) = [(head (show c)), (head (show h))]
    show (Circle c _) = [toLower (head (show c))]
    show (Arrow h _) = [(head (show h))]


-- O valoare din nivel este de tip (Posibil obiect, posibil obiect), unde primul element este
-- un patrat, iar al 2lea este circle sau arrow
data Level = Level (M.Map Position (Maybe Object, Maybe Object))
    deriving (Eq, Ord)


-- Determin coloana minima sau maxiam dintr-o lista de pozitii
minOrMaxCol :: [(Int, Int)] -> (Int -> Int -> Int) -> Int -> Int
minOrMaxCol ((_, col):xs) maxOrMin defaultValue = maxOrMin col (minOrMaxCol xs maxOrMin defaultValue)
minOrMaxCol _ _ defaultValue = defaultValue

-- Determin linia minima sau maxiam dintr-o lista de pozitii
minOrMaxLine :: [(Int, Int)] -> (Int -> Int -> Int) -> Int -> Int
minOrMaxLine ((line, _):xs) maxOrMin defaultValue = maxOrMin line (minOrMaxLine xs maxOrMin defaultValue)
minOrMaxLine _ _ defaultValue = defaultValue

-- current position, endingCol, endingLine, Level
-- aici se printeaza pozitia respectiva din nivel. daca exista, printeaza obiectele, daca nu, atunci spatiu
showObjectAt :: (Int, Int) -> Int -> Int -> Level -> [Char]
showObjectAt (linie, col) endingCol endingLine lev@(Level m) = 
    (if M.notMember (linie, col) m
      then "   "
      else (printAt (linie, col) lev)) ++ 
        (if col == endingCol 
            then (if linie == endingLine then "" else "\n") 
            else "|")

printAt :: (Int, Int) -> Level -> [Char]
printAt pos (Level m) = printBothObjs (M.findWithDefault (Nothing, Nothing) pos m)

printBothObjs :: (Maybe Object, Maybe Object) -> [Char]
printBothObjs (obj1, obj2) = (case obj1 of
                                Nothing -> "  "
                                Just square -> (show square)) ++
                             (case obj2 of
                                Nothing -> " "
                                Just other -> (show other))

instance Show Level where
    show lev@(Level m) =
     if M.null m
        then ""
        else concat (map (\currentPos -> showObjectAt currentPos endingCol endingLine lev) 
                [(a, b) | a <- [startingLine..endingLine], b <- [startingCol..endingCol]])
            where
                 startingCol = minOrMaxCol (M.keys m) min (snd (fst (M.elemAt 0 m)))
                 endingCol = minOrMaxCol (M.keys m) max (snd (fst (M.elemAt 0 m)))
                 startingLine = minOrMaxLine (M.keys m) min (fst (fst (M.elemAt 0 m)))
                 endingLine = minOrMaxLine (M.keys m) max (fst (fst (M.elemAt 0 m)))



-- returneaza obiectele de la pozitia ceruta dintr-un nivel
pairAtMap :: Position -> (M.Map Position (Maybe Object, Maybe Object)) -> (Maybe Object, Maybe Object)
pairAtMap pos m = M.findWithDefault (Nothing, Nothing) pos m

-- la functiile de add se conserva celalalt element. ramane cum era inainte
addSquare :: Color -> Heading -> Position -> Level -> Level
addSquare col heading pos (Level m) = Level (M.insert pos (Just (Square col heading pos), otherObj) m)
                                        where (_, otherObj) = pairAtMap pos m


addCircle :: Color -> Position -> Level -> Level
addCircle col pos (Level m) = Level (M.insert pos (otherObj, Just (Circle col pos)) m)
                                where (otherObj, _) = pairAtMap pos m

addArrow :: Heading -> Position -> Level -> Level
addArrow heading pos (Level m) = Level (M.insert pos (otherObj, Just (Arrow heading pos)) m)
                                    where (otherObj, _) = pairAtMap pos m


posPlusDirection :: Position -> Heading -> Position
posPlusDirection (linie, col) direction =
    case direction of
     North -> (linie - 1, col)
     South -> (linie + 1, col)
     East -> (linie, col + 1)
     West -> (linie, col - 1)

-- se elimina patratul din pozitia respectiva si se apeleaza "punerea" patratului in pozitia viitoare
moveTheSquare :: Position -> Object -> Heading -> Level -> Level
moveTheSquare pos movedSquare direction (Level m) = 
    placeTheSquare (posPlusDirection pos direction) movedSquare direction 
    levWithErase
        where (_, otherObj) = pairAtMap pos m
              levWithErase = case otherObj of
                            Nothing -> Level (M.delete pos m)
                            _ -> Level (M.insert pos (Nothing, otherObj) m)


-- returneaza heading-ul unui patrat. atunci cand plasam un patrat si e posibil sa fie plasat pe un arrow
inheritHeading :: Object -> Maybe Object -> Heading -> Heading
inheritHeading (Square _ _ _)  (Just (Arrow newHeading _)) _ = newHeading
inheritHeading _ _ defaultHeading = defaultHeading

-- se pune patratul pe pozitia respectiva (daca exista deja un patrat unde trebuie sa fie pus acesta,
-- atunci se va muta acel patrat inainte sa fie mutat acesta)
placeTheSquare :: Position -> Object -> Heading -> Level -> Level
placeTheSquare atPos placedSq@(Square col heading _) directionMoved lev@(Level m) =
    case existingSquare of
    Just square -> addSquare col heading atPos (moveTheSquare atPos square directionMoved lev)
    _ -> addSquare col (inheritHeading placedSq existingObj heading) atPos lev
    where (existingSquare, existingObj) = pairAtMap atPos m

placeTheSquare _ _ _ lev = lev


move :: Position  -- Poziția
     -> Level     -- Nivelul inițial
     -> Level     -- Nivelul final
move pos lev@(Level m) = case existingSquare of
                        Just square@(Square _ dir _) -> moveTheSquare pos square dir lev
                        _ -> lev
                     where (existingSquare, _) = pairAtMap pos m


instance ProblemState Level Position where
    successors lev@(Level m) = [(moveDone, levelCreated) | moveDone <- squarePositions, let levelCreated = move moveDone lev]
                                where squarePositions = M.foldr f [] m
                                                        where f = (\(possibleSquare, _) acc ->
                                                                    case possibleSquare of
                                                                        Just (Square _ _ pos) -> pos : acc
                                                                        _ -> acc)

    isGoal (Level m) = M.foldr f True m
                        where f = (\(possibleSquare, secondObj) acc -> 
                                 if acc then
                                        case possibleSquare of 
                                            Just (Square col _ _) -> case secondObj of
                                                                        Just (Circle colCircle _) -> col == colCircle
                                                                        _ -> False
                                            _ -> acc
                                 else False)

    -- Doar petru BONUS
    -- heuristic =
