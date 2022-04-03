package com.games.starwars.view.textures.ships;

import com.games.starwars.view.textures.TexturePack;
import javafx.scene.layout.Pane;

public class ExterminatorTexture extends StarDestroyerTexture implements ShipTexture {

    @Override
    protected void setFill() {
        getTexture().setFill(TexturePack.getExtStarShipTexture(getShip().getCurrentDirection()));
    }

}
